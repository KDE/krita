/*
 * Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _RANSAC_H_

#include <iostream>
#include <list>
#include <vector>

template<class _Tmodel_, class _TmodelStaticParam_, class _Tdata_>
class Ransac {
  public:
    /**
     * @param nbFit number of point to fit the model
     * @param nbGood
     * @param iter number of iterations
     */
    Ransac( uint _nbGood, uint _iter, _TmodelStaticParam_* params) :
      m_nbGood(_nbGood), m_iter(_iter), m_params(params)
    {
      
    }
    ~Ransac()
    {
        delete m_params;
    }
    /**
     */
    std::list<_Tmodel_*> findModels(const std::vector<_Tdata_>& _data )
    {
      std::list<_Tmodel_*> models;
      if( _data.size() < _Tmodel_::nbFit())
      {
        std::cerr << "Not enought data a minimum of " << _Tmodel_::nbFit() << " is required but " << _data.size() << " werer provided" << std::endl;
        return models;
      }
      for(uint i = 0; i < m_iter; i++)
      {
        std::cout << "Iteration " << i << " out of " << m_iter << std::endl;
        std::vector<_Tdata_> samples;
        // Initialize a random samples list
        for (uint is = 0 ; is < _Tmodel_::nbFit() ; ) {
          int idx = (int)( ((double)rand() * _data.size()) / RAND_MAX );
//           std::cerr << idx << " " << _data.size() << std::endl;
          _Tdata_ sample = _data[ idx ];
          if(std::find(samples.begin(), samples.end(), sample) == samples.end())
          {
            samples.push_back(sample);
            ++is;
          }
        }
        // Create a model and see if the samples fit
        _Tmodel_* model = new _Tmodel_( samples, m_params);
        if( model->isValid() )
        { // The model is valid
          // Check non-sample data for points that fit the model
          std::vector<_Tdata_> gooddata;
          double fittingerror = 0.0;
          for( typename std::vector<_Tdata_>::const_iterator it = _data.begin();
               it != _data.end(); ++it)
          {
            if( std::find(samples.begin(), samples.end(), *it) == samples.end())
            { // Not a sample
              double error = model->fittingError( *it ); // Check fitting error
//               std::cerr << error << std::endl;
              if( error < model->threshold() )
              {
                gooddata.push_back( *it );
                fittingerror += error;
              }
            }
          }
          // Check if there is enought good data
          if( gooddata.size() > m_nbGood)
          {
            model->addData( gooddata.begin(), gooddata.end() );
            if(model->isValid())
            {
              models.push_back( model );
            } else {
              delete model;
            }
          }
        } else {
          // Invalid model delete it
          delete model;
        }
      }
      // Sort the models list
      models.sort( compModel);
      return models;
    }
  private:
    static bool compModel(_Tmodel_* m1, _Tmodel_* m2)
    {
      return m1->fittingErrorSum() < m2->fittingErrorSum();
    }
  private:
    uint m_nbGood, m_iter;
    _TmodelStaticParam_* m_params;
};


#endif
