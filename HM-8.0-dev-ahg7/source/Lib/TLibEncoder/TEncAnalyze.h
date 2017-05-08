/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEncAnalyze.h
    \brief    encoder analyzer class (header)
*/

#ifndef __TENCANALYZE__
#define __TENCANALYZE__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComChromaFormat.h"
#include "math.h"

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder analyzer class
class TEncAnalyze
{
private:
  Double    m_dPSNRSum[MAX_NUM_COMPONENT];
  Double    m_dAddBits;
  UInt      m_uiNumPic;
  Double    m_dFrmRate; //--CFG_KDY
  Double    m_MSEyuvframe[MAX_NUM_COMPONENT]; // sum of MSEs
  
public:
  virtual ~TEncAnalyze()  {}
  TEncAnalyze() { clear(); }
  
  Void  addResult( Double psnr[MAX_NUM_COMPONENT], Double bits, const Double MSEyuvframe[MAX_NUM_COMPONENT])
  {
    m_dAddBits  += bits;
    for(UInt i=0; i<MAX_NUM_COMPONENT; i++)
    {
      m_dPSNRSum[i] += psnr[i];
      m_MSEyuvframe[i] += MSEyuvframe[i];
    }

    m_uiNumPic++;
  }
  
  Double  getPsnr(ComponentID compID) const { return  m_dPSNRSum[compID];  }
  Double  getBits()                   const { return  m_dAddBits;   }
  UInt    getNumPic()                 const { return  m_uiNumPic;   }
  
  Void    setFrmRate  (Double dFrameRate) { m_dFrmRate = dFrameRate; } //--CFG_KDY
  Void    clear()
  {
    m_dAddBits = 0;
    for(UInt i=0; i<MAX_NUM_COMPONENT; i++)
    {
      m_dPSNRSum[i] = 0;
      m_MSEyuvframe[i] = 0;
    }
    m_uiNumPic = 0;
  }


  Double calculateCombinedSNR(const ChromaFormat chFmt, const UInt maxval)
  {
    Double MSEyuv=0;
    Int scale=0;
    const UInt numberValidComponents = getNumberValidComponents(chFmt);
    for (UInt comp=0; comp<numberValidComponents; comp++)
    {
      const ComponentID compID = ComponentID(comp);
      const UInt csx = getComponentScaleX(compID, chFmt);
      const UInt csy = getComponentScaleY(compID, chFmt);
      Int scaleChan  = (4>>(csx+csy));
      scale         += scaleChan;
      MSEyuv        += scaleChan * (m_MSEyuvframe[compID]/Double(getNumPic()));
    }

    MSEyuv /= Double(scale);  // i.e. divide by 6 for 4:2:0, 8 for 4:2:2 etc.
    Double PSNRyuv = (MSEyuv==0 ? 99.99 : 10*log10((maxval*maxval)/MSEyuv));
    return PSNRyuv;
  }


  Void    printOut ( Char cDelim, const ChromaFormat chFmt, const UInt maxval )
  {
    Double dFps     =   m_dFrmRate; //--CFG_KDY
    Double dScale   = dFps / 1000 / (Double)m_uiNumPic;

    switch (chFmt)
    {
      case CHROMA_400:
        printf( "\tTotal Frames |  "   "Bitrate    "  "Y-PSNR\n" );
        //printf( "\t------------ "  " ----------"   " -------- "  " -------- "  " --------\n" );
        printf( "\t %8d    %c"          "%12.4lf  "    "%8.4lf\n",
               getNumPic(), cDelim,
               getBits() * dScale,
               getPsnr(COMPONENT_Y) / (Double)getNumPic() );
        break;
      case CHROMA_420:
      case CHROMA_422:
      case CHROMA_444:
        {
          Double PSNRyuv = calculateCombinedSNR(chFmt, maxval);
          printf( "\tTotal Frames |  "   "Bitrate    "  "Y-PSNR    "  "U-PSNR    "  "V-PSNR    "  "YUV-PSNR \n" );
          //printf( "\t------------ "  " ----------"   " -------- "  " -------- "  " --------\n" );
          printf( "\t %8d    %c"          "%12.4lf  "    "%8.4lf  "   "%8.4lf  "    "%8.4lf  "   "%8.4lf\n",
                 getNumPic(), cDelim,
                 getBits() * dScale,
                 getPsnr(COMPONENT_Y) / (Double)getNumPic(),
                 getPsnr(COMPONENT_Cb) / (Double)getNumPic(),
                 getPsnr(COMPONENT_Cr) / (Double)getNumPic(),
                 PSNRyuv );
          break;
        }
      default:
        fprintf(stderr, "Unknown format during print out\n");
        exit(1);
        break;
    }
  }
  

  Void    printSummary(const ChromaFormat chFmt, const UInt maxval, Char ch='T')
  {
    FILE* pFile = NULL;
    
    switch( ch ) 
    {
      case 'T':
        pFile = fopen ("summaryTotal.txt", "at");
        break;
      case 'I':
        pFile = fopen ("summary_I.txt", "at");
        break;
      case 'P':
        pFile = fopen ("summary_P.txt", "at");
        break;
      case 'B':
        pFile = fopen ("summary_B.txt", "at");
        break;
      default:
        assert(0);
        return;
        break;
    }
    
    Double dFps     =   m_dFrmRate; //--CFG_KDY
    Double dScale   = dFps / 1000 / (Double)m_uiNumPic;
    switch (chFmt)
    {
      case CHROMA_400:
        fprintf(pFile, "%f\t %f\n",
            getBits() * dScale,
            getPsnr(COMPONENT_Y) / (Double)getNumPic() );
        break;
      case CHROMA_420:
      case CHROMA_422:
      case CHROMA_444:
        {
          Double PSNRyuv = calculateCombinedSNR(chFmt, maxval);
          fprintf(pFile, "%f\t %f\t %f\t %f\t %f\n",
              getBits() * dScale,
              getPsnr(COMPONENT_Y) / (Double)getNumPic(),
              getPsnr(COMPONENT_Cb) / (Double)getNumPic(),
              getPsnr(COMPONENT_Cr) / (Double)getNumPic(),
              PSNRyuv );
          break;
        }

      default:
          fprintf(stderr, "Unknown format during print out\n");
          exit(1);
          break;
    }
    
    fclose(pFile);
  }
};

extern TEncAnalyze             m_gcAnalyzeAll;
extern TEncAnalyze             m_gcAnalyzeI;
extern TEncAnalyze             m_gcAnalyzeP;
extern TEncAnalyze             m_gcAnalyzeB;

//! \}

#endif // !defined(AFX_TENCANALYZE_H__C79BCAA2_6AC8_4175_A0FE_CF02F5829233__INCLUDED_)
