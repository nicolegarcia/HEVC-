/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2017, ITU/ISO/IEC
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

/** \file     TComYuv.cpp
    \brief    general YUV buffer class
    \todo     this should be merged with TComPicYuv
*/

#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <math.h>

#include "CommonDef.h"
#include "TComYuv.h"
#include "TComInterpolationFilter.h"

//! \ingroup TLibCommon
//! \{

TComYuv::TComYuv()
{
  for(Int comp=0; comp<MAX_NUM_COMPONENT; comp++)
  {
    m_apiBuf[comp] = NULL;
  }
}

TComYuv::~TComYuv()
{
  destroy();
}

Void TComYuv::create( UInt iWidth, UInt iHeight, ChromaFormat chromaFormatIDC )
{
  destroy();
  // set width and height
  m_iWidth   = iWidth;
  m_iHeight  = iHeight;
  m_chromaFormatIDC = chromaFormatIDC;

  for(Int comp=0; comp<MAX_NUM_COMPONENT; comp++)
  {
    // memory allocation
    m_apiBuf[comp]  = (Pel*)xMalloc( Pel, getWidth(ComponentID(comp))*getHeight(ComponentID(comp)) );
  }
}

Void TComYuv::destroy()
{
  // memory free
  for(Int comp=0; comp<MAX_NUM_COMPONENT; comp++)
  {
    if (m_apiBuf[comp]!=NULL)
    {
      xFree( m_apiBuf[comp] );
      m_apiBuf[comp] = NULL;
    }
  }
}

Void TComYuv::clear()
{
  for(Int comp=0; comp<MAX_NUM_COMPONENT; comp++)
  {
    if (m_apiBuf[comp]!=NULL)
    {
      ::memset( m_apiBuf[comp], 0, ( getWidth(ComponentID(comp)) * getHeight(ComponentID(comp))  )*sizeof(Pel) );
    }
  }
}




Void TComYuv::copyToPicYuv   ( TComPicYuv* pcPicYuvDst, const UInt ctuRsAddr, const UInt uiAbsZorderIdx, const UInt uiPartDepth, const UInt uiPartIdx ) const
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    copyToPicComponent  ( ComponentID(comp), pcPicYuvDst, ctuRsAddr, uiAbsZorderIdx, uiPartDepth, uiPartIdx );
  }
}

Void TComYuv::copyToPicComponent  ( const ComponentID compID, TComPicYuv* pcPicYuvDst, const UInt ctuRsAddr, const UInt uiAbsZorderIdx, const UInt uiPartDepth, const UInt uiPartIdx ) const
{
  const Int iWidth  = getWidth(compID) >>uiPartDepth;
  const Int iHeight = getHeight(compID)>>uiPartDepth;

  const Pel* pSrc     = getAddr(compID, uiPartIdx, iWidth);
        Pel* pDst     = pcPicYuvDst->getAddr ( compID, ctuRsAddr, uiAbsZorderIdx );

  const UInt  iSrcStride  = getStride(compID);
  const UInt  iDstStride  = pcPicYuvDst->getStride(compID);

  for ( Int y = iHeight; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*iWidth);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}




Void TComYuv::copyFromPicYuv   ( const TComPicYuv* pcPicYuvSrc, const UInt ctuRsAddr, const UInt uiAbsZorderIdx )
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    copyFromPicComponent  ( ComponentID(comp), pcPicYuvSrc, ctuRsAddr, uiAbsZorderIdx );
  }
}

Void TComYuv::copyFromPicComponent  ( const ComponentID compID, const TComPicYuv* pcPicYuvSrc, const UInt ctuRsAddr, const UInt uiAbsZorderIdx )
{
        Pel* pDst     = getAddr(compID);
  const Pel* pSrc     = pcPicYuvSrc->getAddr ( compID, ctuRsAddr, uiAbsZorderIdx );

  const UInt iDstStride  = getStride(compID);
  const UInt iSrcStride  = pcPicYuvSrc->getStride(compID);
  const Int  iWidth=getWidth(compID);
  const Int  iHeight=getHeight(compID);

  for (Int y = iHeight; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*iWidth);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}




Void TComYuv::copyToPartYuv( TComYuv* pcYuvDst, const UInt uiDstPartIdx ) const
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    copyToPartComponent  ( ComponentID(comp), pcYuvDst, uiDstPartIdx );
  }
}

Void TComYuv::copyToPartComponent( const ComponentID compID, TComYuv* pcYuvDst, const UInt uiDstPartIdx ) const
{
  const Pel* pSrc     = getAddr(compID);
        Pel* pDst     = pcYuvDst->getAddr( compID, uiDstPartIdx );

  const UInt iSrcStride  = getStride(compID);
  const UInt iDstStride  = pcYuvDst->getStride(compID);
  const Int  iWidth=getWidth(compID);
  const Int  iHeight=getHeight(compID);

  for (Int y = iHeight; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*iWidth);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}




Void TComYuv::copyPartToYuv( TComYuv* pcYuvDst, const UInt uiSrcPartIdx ) const
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    copyPartToComponent  ( ComponentID(comp), pcYuvDst, uiSrcPartIdx );
  }
}

Void TComYuv::copyPartToComponent( const ComponentID compID, TComYuv* pcYuvDst, const UInt uiSrcPartIdx ) const
{
  const Pel* pSrc     = getAddr(compID, uiSrcPartIdx);
        Pel* pDst     = pcYuvDst->getAddr(compID, 0 );

  const UInt  iSrcStride  = getStride(compID);
  const UInt  iDstStride  = pcYuvDst->getStride(compID);

  const UInt uiHeight = pcYuvDst->getHeight(compID);
  const UInt uiWidth = pcYuvDst->getWidth(compID);

  for ( UInt y = uiHeight; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*uiWidth);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}




Void TComYuv::copyPartToPartYuv   ( TComYuv* pcYuvDst, const UInt uiPartIdx, const UInt iWidth, const UInt iHeight ) const
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    copyPartToPartComponent   (ComponentID(comp), pcYuvDst, uiPartIdx, iWidth>>getComponentScaleX(ComponentID(comp)), iHeight>>getComponentScaleY(ComponentID(comp)) );
  }
}

Void TComYuv::copyPartToPartComponent  ( const ComponentID compID, TComYuv* pcYuvDst, const UInt uiPartIdx, const UInt iWidthComponent, const UInt iHeightComponent ) const
{
  const Pel* pSrc =           getAddr(compID, uiPartIdx);
        Pel* pDst = pcYuvDst->getAddr(compID, uiPartIdx);
  if( pSrc == pDst )
  {
    //th not a good idea
    //th best would be to fix the caller
    return ;
  }

  const UInt  iSrcStride = getStride(compID);
  const UInt  iDstStride = pcYuvDst->getStride(compID);
  for ( UInt y = iHeightComponent; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, iWidthComponent * sizeof(Pel) );
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
}




Void TComYuv::copyPartToPartComponentMxN  ( const ComponentID compID, TComYuv* pcYuvDst, const TComRectangle &rect) const
{
  const Pel* pSrc =           getAddrPix( compID, rect.x0, rect.y0 );
        Pel* pDst = pcYuvDst->getAddrPix( compID, rect.x0, rect.y0 );
  if( pSrc == pDst )
  {
    //th not a good idea
    //th best would be to fix the caller
    return ;
  }

  const UInt  iSrcStride = getStride(compID);
  const UInt  iDstStride = pcYuvDst->getStride(compID);
  const UInt uiHeightComponent=rect.height;
  const UInt uiWidthComponent=rect.width;
  for ( UInt y = uiHeightComponent; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, uiWidthComponent * sizeof( Pel ) );
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
}




Void TComYuv::addClip( const TComYuv* pcYuvSrc0, const TComYuv* pcYuvSrc1, const UInt uiTrUnitIdx, const UInt uiPartSize, const BitDepths &clipBitDepths )
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    const ComponentID compID=ComponentID(comp);
    const Int uiPartWidth =uiPartSize>>getComponentScaleX(compID);
    const Int uiPartHeight=uiPartSize>>getComponentScaleY(compID);

    const Pel* pSrc0 = pcYuvSrc0->getAddr(compID, uiTrUnitIdx, uiPartWidth );
    const Pel* pSrc1 = pcYuvSrc1->getAddr(compID, uiTrUnitIdx, uiPartWidth );
          Pel* pDst  = getAddr(compID, uiTrUnitIdx, uiPartWidth );

    const UInt iSrc0Stride = pcYuvSrc0->getStride(compID);
    const UInt iSrc1Stride = pcYuvSrc1->getStride(compID);
    const UInt iDstStride  = getStride(compID);
    const Int clipbd = clipBitDepths.recon[toChannelType(compID)];
#if O0043_BEST_EFFORT_DECODING
    const Int bitDepthDelta = clipBitDepths.stream[toChannelType(compID)] - clipbd;
#endif

    for ( Int y = uiPartHeight-1; y >= 0; y-- )
    {
      for ( Int x = uiPartWidth-1; x >= 0; x-- )
      {
#if O0043_BEST_EFFORT_DECODING
        pDst[x] = Pel(ClipBD<Int>( Int(pSrc0[x]) + rightShiftEvenRounding<Pel>(pSrc1[x], bitDepthDelta), clipbd));
#else
        pDst[x] = Pel(ClipBD<Int>( Int(pSrc0[x]) + Int(pSrc1[x]), clipbd));
#endif
      }
      pSrc0 += iSrc0Stride;
      pSrc1 += iSrc1Stride;
      pDst  += iDstStride;
    }
  }
}




Void TComYuv::subtract( const TComYuv* pcYuvSrc0, const TComYuv* pcYuvSrc1, const UInt uiTrUnitIdx, const UInt uiPartSize )
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    const ComponentID compID=ComponentID(comp);
    const Int uiPartWidth =uiPartSize>>getComponentScaleX(compID);
    const Int uiPartHeight=uiPartSize>>getComponentScaleY(compID);

    const Pel* pSrc0 = pcYuvSrc0->getAddr( compID, uiTrUnitIdx, uiPartWidth );
    const Pel* pSrc1 = pcYuvSrc1->getAddr( compID, uiTrUnitIdx, uiPartWidth );
          Pel* pDst  = getAddr( compID, uiTrUnitIdx, uiPartWidth );

    const Int  iSrc0Stride = pcYuvSrc0->getStride(compID);
    const Int  iSrc1Stride = pcYuvSrc1->getStride(compID);
    const Int  iDstStride  = getStride(compID);

    for (Int y = uiPartHeight-1; y >= 0; y-- )
    {
      for (Int x = uiPartWidth-1; x >= 0; x-- )
      {
        pDst[x] = pSrc0[x] - pSrc1[x];
      }
      pSrc0 += iSrc0Stride;
      pSrc1 += iSrc1Stride;
      pDst  += iDstStride;
    }
  }
}




Void TComYuv::addAvg( const TComYuv* pcYuvSrc0, const TComYuv* pcYuvSrc1, const UInt iPartUnitIdx, const UInt uiWidth, const UInt uiHeight, const BitDepths &clipBitDepths )
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    const ComponentID compID=ComponentID(comp);
    const Pel* pSrc0  = pcYuvSrc0->getAddr( compID, iPartUnitIdx );
    const Pel* pSrc1  = pcYuvSrc1->getAddr( compID, iPartUnitIdx );
    Pel* pDst   = getAddr( compID, iPartUnitIdx );

    const UInt  iSrc0Stride = pcYuvSrc0->getStride(compID);
    const UInt  iSrc1Stride = pcYuvSrc1->getStride(compID);
    const UInt  iDstStride  = getStride(compID);
    const Int   clipbd      = clipBitDepths.recon[toChannelType(compID)];
    const Int   shiftNum    = std::max<Int>(2, (IF_INTERNAL_PREC - clipbd)) + 1;
    const Int   offset      = ( 1 << ( shiftNum - 1 ) ) + 2 * IF_INTERNAL_OFFS;

    const Int   iWidth      = uiWidth  >> getComponentScaleX(compID);
    const Int   iHeight     = uiHeight >> getComponentScaleY(compID);

    if (iWidth&1)
    {
      assert(0);
      exit(-1);
    }
    else if (iWidth&2)
    {
      for ( Int y = 0; y < iHeight; y++ )
      {
        for (Int x=0 ; x < iWidth; x+=2 )
        {
          pDst[ x + 0 ] = ClipBD( rightShift(( pSrc0[ x + 0 ] + pSrc1[ x + 0 ] + offset ), shiftNum), clipbd );
          pDst[ x + 1 ] = ClipBD( rightShift(( pSrc0[ x + 1 ] + pSrc1[ x + 1 ] + offset ), shiftNum), clipbd );
        }
        pSrc0 += iSrc0Stride;
        pSrc1 += iSrc1Stride;
        pDst  += iDstStride;
      }
    }
    else
    {
      for ( Int y = 0; y < iHeight; y++ )
      {
        for (Int x=0 ; x < iWidth; x+=4 )
        {
          pDst[ x + 0 ] = ClipBD( rightShift(( pSrc0[ x + 0 ] + pSrc1[ x + 0 ] + offset ), shiftNum), clipbd );
          pDst[ x + 1 ] = ClipBD( rightShift(( pSrc0[ x + 1 ] + pSrc1[ x + 1 ] + offset ), shiftNum), clipbd );
          pDst[ x + 2 ] = ClipBD( rightShift(( pSrc0[ x + 2 ] + pSrc1[ x + 2 ] + offset ), shiftNum), clipbd );
          pDst[ x + 3 ] = ClipBD( rightShift(( pSrc0[ x + 3 ] + pSrc1[ x + 3 ] + offset ), shiftNum), clipbd );
        }
        pSrc0 += iSrc0Stride;
        pSrc1 += iSrc1Stride;
        pDst  += iDstStride;
      }
    }
  }
}

Void TComYuv::removeHighFreq( const TComYuv* pcYuvSrc,
                              const UInt uiPartIdx,
                              const UInt uiWidth,
                              const UInt uiHeight,
                              const Int bitDepths[MAX_NUM_CHANNEL_TYPE],
                              const Bool bClipToBitDepths
                              )
{
  for(Int comp=0; comp<getNumberValidComponents(); comp++)
  {
    const ComponentID compID=ComponentID(comp);
    const Pel* pSrc  = pcYuvSrc->getAddr(compID, uiPartIdx);
    Pel* pDst  = getAddr(compID, uiPartIdx);

    const Int iSrcStride = pcYuvSrc->getStride(compID);
    const Int iDstStride = getStride(compID);
    const Int iWidth  = uiWidth >>getComponentScaleX(compID);
    const Int iHeight = uiHeight>>getComponentScaleY(compID);
    if (bClipToBitDepths)
    {
      const Int clipBd=bitDepths[toChannelType(compID)];
      for ( Int y = iHeight-1; y >= 0; y-- )
      {
        for ( Int x = iWidth-1; x >= 0; x-- )
        {
          pDst[x ] = ClipBD((2 * pDst[x]) - pSrc[x], clipBd);
        }
        pSrc += iSrcStride;
        pDst += iDstStride;
      }
    }
    else
    {
      for ( Int y = iHeight-1; y >= 0; y-- )
      {
        for ( Int x = iWidth-1; x >= 0; x-- )
        {
          pDst[x ] = (2 * pDst[x]) - pSrc[x];
        }
        pSrc += iSrcStride;
        pDst += iDstStride;
      }
    }
  }
}

Void TComYuv::convert(const Bool extendedPrecision, const UInt pixX, const UInt pixY, const UInt width, Bool bForwardConversion, const BitDepths& bitDepths, Bool bLossless, TComYuv* pcYuvNoCorrResi)
{
  const Int lumaBitDepth   = bitDepths.recon[CHANNEL_TYPE_LUMA];
  const Int chromaBitDepth = bitDepths.recon[CHANNEL_TYPE_CHROMA];

  Int CoeffMinY = -(1<<(extendedPrecision? std::max(15, lumaBitDepth+6):15));
  Int CoeffMinC = -(1<<(extendedPrecision? std::max(15, chromaBitDepth+6):15));
  Int CoeffMaxY =  (1<<(extendedPrecision? std::max(15, lumaBitDepth+6):15)) - 1;
  Int CoeffMaxC =  (1<<(extendedPrecision? std::max(15, chromaBitDepth+6):15)) - 1;
  assert(getChromaFormat() == CHROMA_444);
  UInt partSize = width;

  Pel* pOrg0  = getAddrPix( (ComponentID)0, pixX, pixY );
  Pel* pOrg1  = getAddrPix( (ComponentID)1, pixX, pixY );
  Pel* pOrg2  = getAddrPix( (ComponentID)2, pixX, pixY );

  Pel* pDst0  = getAddrPix( (ComponentID)0, pixX, pixY );
  Pel* pDst1  = getAddrPix( (ComponentID)1, pixX, pixY );
  Pel* pDst2  = getAddrPix( (ComponentID)2, pixX, pixY );

  if(pcYuvNoCorrResi)
  {
    pDst0  = pcYuvNoCorrResi->getAddrPix( (ComponentID)0, pixX, pixY );
    pDst1  = pcYuvNoCorrResi->getAddrPix( (ComponentID)1, pixX, pixY );
    pDst2  = pcYuvNoCorrResi->getAddrPix( (ComponentID)2, pixX, pixY );
  }
  const Int  iStride0 = getStride((ComponentID)0);
  const Int  iStride1 = getStride((ComponentID)1);
  const Int  iStride2 = getStride((ComponentID)2);

  if(bForwardConversion)
  {
    if(!bLossless)
    {
      Int maxBitDepth  = std::max(bitDepths.recon[CHANNEL_TYPE_LUMA], bitDepths.recon[CHANNEL_TYPE_CHROMA]);
      Int iShiftLuma   = maxBitDepth - bitDepths.recon[CHANNEL_TYPE_LUMA];
      Int iShiftChroma = maxBitDepth - bitDepths.recon[CHANNEL_TYPE_CHROMA];
      Int iRoundLuma   = 1<<(1+iShiftLuma);
      Int iRoundChroma = 1<<(1+iShiftChroma);
      for(Int y=0; y<partSize; y++)
      {
        for(Int x=0; x<partSize; x++)
        {
          Int r, g, b;
          r = pOrg2[x]<<iShiftChroma;
          g = pOrg0[x]<<iShiftLuma;
          b = pOrg1[x]<<iShiftChroma;

          pDst0[x] = (g<<1) +r+b  ;
          pDst1[x] = (g<<1) -r-b  ;
          pDst2[x] = ((r-b) << 1) ;
          pDst0[x] = (pDst0[x] + iRoundLuma)   >> (2+iShiftLuma);
          pDst1[x] = (pDst1[x] + iRoundChroma) >> (2+iShiftChroma);
          pDst2[x] = (pDst2[x] + iRoundChroma) >> (2+iShiftChroma);
        }
        pOrg0 += iStride0;
        pOrg1 += iStride1;
        pOrg2 += iStride2;
        pDst0 += iStride0;
        pDst1 += iStride1;
        pDst2 += iStride2;
      }
    }
    else
    {
      for(Int y=0; y<partSize; y++)
      {
        for(Int x=0; x<partSize; x++)
        {
          Int r, g, b;
          r = pOrg2[x];
          g = pOrg0[x];
          b = pOrg1[x];

          Int Co = r-b;
          Int t = b + (Co>>1);
          Int Cg = g - t;
          pDst0[x] = t + (Cg>>1);
          pDst1[x] = Cg;
          pDst2[x] = Co;
        }
        pOrg0 += iStride0;
        pOrg1 += iStride1;
        pOrg2 += iStride2;
        pDst0 += iStride0;
        pDst1 += iStride1;
        pDst2 += iStride2;
      }
    }
  }
  else
  {
    Int maxBitDepth  = std::max(bitDepths.recon[CHANNEL_TYPE_LUMA], bitDepths.recon[CHANNEL_TYPE_CHROMA]);
    Int iShiftLuma   = maxBitDepth - bitDepths.recon[CHANNEL_TYPE_LUMA];
    Int iShiftChroma = maxBitDepth - bitDepths.recon[CHANNEL_TYPE_CHROMA];
    Int iRoundLuma   = iShiftLuma? (1<<(iShiftLuma-1)): 0;
    Int iRoundChroma = iShiftChroma? (1<<(iShiftChroma-1)): 0;
    for(Int y=0; y<partSize; y++)
    {
      for(Int x=0; x<partSize; x++)
      {
        Int y0, cg, co;
        y0 = Clip3<Int>(CoeffMinY,CoeffMaxY, pOrg0[x]);
        cg = Clip3<Int>(CoeffMinC,CoeffMaxC, pOrg1[x]);
        co = Clip3<Int>(CoeffMinC,CoeffMaxC, pOrg2[x]);

        if(!bLossless)
        {
          y0 <<= iShiftLuma;
          cg <<= (iShiftChroma + 1);
          co <<= (iShiftChroma + 1);
        }
        Int t = y0 - (cg>>1);
        pDst0[x] = cg + t;
        pDst1[x] = t - (co>>1);
        pDst2[x] = co + pDst1[x];
        if(!bLossless)
        {
          pDst0[x] = (pDst0[x] + iRoundLuma)   >> iShiftLuma;
          pDst1[x] = (pDst1[x] + iRoundChroma) >> iShiftChroma;
          pDst2[x] = (pDst2[x] + iRoundChroma) >> iShiftChroma;
        }
      }
      pOrg0 += iStride0;
      pOrg1 += iStride1;
      pOrg2 += iStride2;
      pDst0 += iStride0;
      pDst1 += iStride1;
      pDst2 += iStride2;
    }
  }
}

Void TComYuv::DefaultConvertPix(const UInt pixX, const UInt pixY, const UInt width, const BitDepths& bitDepths )
{
  assert(getChromaFormat() == CHROMA_444);
  UInt partSize = width;
  Int  iMaxLuma   = (1<<bitDepths.recon[CHANNEL_TYPE_LUMA])   - 1;
  Int  iMaxChroma = (1<<bitDepths.recon[CHANNEL_TYPE_CHROMA]) - 1;
  Int  iChromaOffset = (1<<(bitDepths.recon[CHANNEL_TYPE_CHROMA]-1));
  Int maxBitDepth  = std::max(bitDepths.recon[CHANNEL_TYPE_LUMA], bitDepths.recon[CHANNEL_TYPE_CHROMA]);
  Int iShiftLuma   = maxBitDepth - bitDepths.recon[CHANNEL_TYPE_LUMA];
  Int iShiftChroma = maxBitDepth - bitDepths.recon[CHANNEL_TYPE_CHROMA];
  Int iRoundLuma   = 1<<(1+iShiftLuma);
  Int iRoundChroma = 1<<(1+iShiftChroma);

  Pel* pDst0  = getAddrPix( (ComponentID)0, pixX, pixY );
  Pel* pDst1  = getAddrPix( (ComponentID)1, pixX, pixY );
  Pel* pDst2  = getAddrPix( (ComponentID)2, pixX, pixY );

  const Int  iStride0 = getStride((ComponentID)0);
  const Int  iStride1 = getStride((ComponentID)1);
  const Int  iStride2 = getStride((ComponentID)2);

  for(Int y=0; y<partSize; y++)
  {
    for(Int x=0; x<partSize; x++)
    {
      Int r, g, b;
      r = pDst2[x]<<iShiftChroma;
      g = pDst0[x]<<iShiftLuma;
      b = pDst1[x]<<iShiftChroma;

      pDst0[x] = ((g<<1)+r+b + iRoundLuma)>>(2+iShiftLuma);
      pDst1[x] = ((g<<1)-r-b + iRoundChroma)>>(2+iShiftChroma);
      pDst2[x] = (((r-b)<<1)  + iRoundChroma)>>(2+iShiftChroma);

      pDst1[x] += iChromaOffset;
      pDst2[x] += iChromaOffset;
      pDst0[x] = Clip3( 0, iMaxLuma,   Int(pDst0[x]) );
      pDst1[x] = Clip3( 0, iMaxChroma, Int(pDst1[x]) );
      pDst2[x] = Clip3( 0, iMaxChroma, Int(pDst2[x]) );
    }

    pDst0 += iStride0;
    pDst1 += iStride1;
    pDst2 += iStride2;
  }
}

//! \}
