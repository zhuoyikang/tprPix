/*
 * ====================== MapAltitude.cpp =======================
 *                          -- tpr --
 *                                        CREATE -- 2019.03.11
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "MapAltitude.h"

//------------------- Engine --------------------//
#include "tprAssert.h"
#include "config.h"


/* ===========================================================
 *                         set    [2]
 * -----------------------------------------------------------
 */
void MapAltitude::set( float altiVal_from_gpgpu_ ){

    tprAssert( (altiVal_from_gpgpu_>=-100.0f) && (altiVal_from_gpgpu_<=100.0f) );
    this->val = static_cast<int>(altiVal_from_gpgpu_);

    //------------------//
    //      lvl
    //------------------//
    int    tmpLvl    {};
    float waterStep  { 7.0f }; //- 水下梯度，tmp...
    float landStep   { 14.0f }; //- 陆地梯度，tmp...

    if( this->val < 0 ){ //- under water
        tmpLvl = static_cast<int>( floor(altiVal_from_gpgpu_/waterStep) );
        if( tmpLvl < -5 ){ //- 抹平 水域底部
            tmpLvl = -5;
        }
    }else{ //- land
        tmpLvl = static_cast<int>( floor(altiVal_from_gpgpu_/landStep) );
        if( tmpLvl > 2 ){ //- 抹平 陆地高区
            tmpLvl = 2;
        }
    }
    this->lvl = tmpLvl;

}