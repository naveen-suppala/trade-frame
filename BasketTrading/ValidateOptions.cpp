/************************************************************************
 * Copyright(c) 2019, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/* 
 * File:    ValidateOptions.cpp
 * Author:  raymond@burkholder.net
 * Project: BasketTrading
 * Created on July 4, 2019, 12:10 PM
 */

#include "ValidateOptions.h"

ValidateOptions::ValidateOptions( 
  pWatch_t pWatchUnderlying,  // underlying
  const mapChains_t& mapChains, 
  fConstructOption_t& fConstructOption )
: m_pWatchUnderlying( pWatchUnderlying ),
  m_mapChains( mapChains ),
  m_fConstructOption( fConstructOption )
{
  assert( nullptr != fConstructOption );
}

ValidateOptions::ValidateOptions( const ValidateOptions& rhs )
: m_pWatchUnderlying( rhs.m_pWatchUnderlying ),
  m_mapChains( rhs.m_mapChains ),
  m_fConstructOption( rhs.m_fConstructOption ),
  m_SpreadValidation( rhs.m_SpreadValidation )
{
}

ValidateOptions::ValidateOptions( const ValidateOptions&& rhs )
: m_pWatchUnderlying( std::move( rhs.m_pWatchUnderlying ) ),
  m_mapChains( rhs.m_mapChains ),
  m_fConstructOption( rhs.m_fConstructOption ),
  m_SpreadValidation( std::move( rhs.m_SpreadValidation ) )
{
}

ValidateOptions::~ValidateOptions( ) {
  m_SpreadValidation.ResetOptions();
}

void ValidateOptions::SetSize( vLegSelected_t::size_type size ) {
  m_vLegSelected.resize( size );
}

bool ValidateOptions::ValidateSpread( 
  boost::gregorian::date dateToday, double price, size_t nDuration, fChooseStrikes_t&& fChooseStrikes
) {

  double bStrikesFound( false );

  size_t ixLegSelected {};
  try {
    fChooseStrikes( m_mapChains, dateToday, price,
      [this,&ixLegSelected](double strike, boost::gregorian::date dateStrike, const std::string& sIQFeedName){
        m_vLegSelected.at( ixLegSelected ).Update( strike, dateStrike, sIQFeedName );
        ixLegSelected++;
      } );
    bStrikesFound = true; // can set as no exception was thrown
  }
  catch ( const ou::tf::option::exception_strike_range_exceeded& e ) {
    // don't worry about this, price is not with in range yet
    throw e;
  }
  catch ( const std::runtime_error& e ) {
    const std::string& sUnderlying( m_pWatchUnderlying->GetInstrument()->GetInstrumentName() );
    std::cout
      << sUnderlying
      << " found no strike for "
      << " mid-point=" << price
      << ", today=" << dateToday
//        << " for quote " << m_QuoteUnderlyingLatest.DateTime().date()
      << " [" << e.what() << "]"
      << std::endl;
    throw e;
  }

  bool bBuildOptions( false );
  size_t nReason {};

  if ( bStrikesFound ) {
    if ( !m_SpreadValidation.IsActive() ) {
      nReason = 1;
      bBuildOptions = true;
    }
    else {
      bool bAnyChanged( false );
      for ( vLegSelected_t::value_type& vt: m_vLegSelected ) {
        bAnyChanged = ( bAnyChanged || vt.Changed() );
      }
      if ( bAnyChanged ) {  // TODO: refine the following logic to reset only legs requiring change
        m_SpreadValidation.ResetOptions(); // why doesn't this cause a miss on quote stop/start?
        nReason = 2;
        bBuildOptions = true;
      }
    }
  }

  if ( bBuildOptions ) {
    const std::string& sUnderlying( m_pWatchUnderlying->GetInstrument()->GetInstrumentName() );
    std::cout
      << sUnderlying
      << "("
      << nReason
      << "," << m_vLegSelected.size()
      << ")"
      << ": combo -> price=" << price;
    for ( const vLegSelected_t::value_type& vt: m_vLegSelected ) {
      std::cout << ",strike=" << vt.Strike() << "@" << vt.Expiry();
    }
    std::cout << std::endl;

    pInstrument_t pInstrumentUnderlying = m_pWatchUnderlying->GetInstrument();
    
    m_SpreadValidation.SetLegCount( m_vLegSelected.size() );

    size_t n {};
    for ( vLegSelected_t::value_type& vt: m_vLegSelected ) {
//      if ( vt.Changed() || ( 1 == nReason ) ) {
        m_fConstructOption(
          vt.IQFeedName(),
          pInstrumentUnderlying,
          [this,&vt,n]( pOption_t pOption ) {
            vt.Option() = pOption;
            m_SpreadValidation.SetWatch( n, pOption );
          }
          );
//        vt.ResetChanged();
//      }
      n++;
    }
  } // bBuildOptions

    // ==

  bool bValidated( false );
  if ( m_SpreadValidation.IsActive() ) {
    bValidated = m_SpreadValidation.Validate( nDuration );
  }

  return bValidated;
}

void ValidateOptions::ValidatedOptions( fValidatedOption_t&& fValidatedOption ) {
  assert( m_SpreadValidation.IsActive() );
  for ( vLegSelected_t::value_type& vt: m_vLegSelected ) {
    fValidatedOption( vt.Option() );
  }
//  m_SpreadValidation.ResetOptions();
}

void ValidateOptions::ClearValidation() {
  m_SpreadValidation.ResetOptions();
  for ( vLegSelected_t::value_type& vt: m_vLegSelected ) {
    vt.Clear();
  }
}

