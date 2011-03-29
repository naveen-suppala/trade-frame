/************************************************************************
 * Copyright(c) 2009, One Unified. All rights reserved.                 *
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

#pragma once

#include <map>
#include <string>

#include "KeyTypes.h"

#include "ManagerBase.h"
#include "Portfolio.h"

namespace ou { // One Unified
namespace tf { // TradeFrame

class CPortfolioManager: public ManagerBase<CPortfolioManager> {
public:

  typedef CPortfolio::pPortfolio_t pPortfolio_t;
  typedef keytypes::idPortfolio_t idPortfolio_t;

  typedef keytypes::idAccountOwner_t idAccountOwner_t;

  CPortfolioManager(void) {};
  ~CPortfolioManager(void) {};

  pPortfolio_t ConstructPortfolio( 
    const idPortfolio_t& idPortfolio, const idAccountOwner_t& idAccountOwner, const std::string& sDescription = "" );
  pPortfolio_t GetPortfolio( const idPortfolio_t& idPortfolio );
  void UpdatePortfolio( const idPortfolio_t& idPortfolio );
  void DeletePortfolio( const idPortfolio_t& idPortfolio );

  void RegisterTablesForCreation( void );
  void RegisterRowDefinitions( void );
  void PopulateTables( void );

protected:

private:

  typedef std::pair<idPortfolio_t, pPortfolio_t> mapPortfolio_pair_t;
  typedef std::map<idPortfolio_t, pPortfolio_t> mapPortfolio_t;
  typedef mapPortfolio_t::iterator iterPortfolio_t;
  mapPortfolio_t m_mapPortfolio;

};


} // namespace tf
} // namespace ou
