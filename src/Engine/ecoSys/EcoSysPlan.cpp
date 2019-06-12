/*
 * ========================= EcoSysPlan.cpp =======================
 *                          -- tpr --
 *                                        CREATE -- 2019.04.16
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "EcoSysPlan.h"

//-------------------- C --------------------//
#include <cmath>

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "Density.h"
#include "random.h"

//-------------------- Script --------------------//
#include "Script/resource/ssrc.h" 


namespace ecoSysPlan_inn {//-------- namespace: ecoSysPlan_inn --------------//

    //- 根据 density.lvl, 颜色递进（逐渐加深）
    int off_r  { -11 };
    int off_g  { -8 };
    int off_b  { -7 };

}//------------- namespace: ecoSysPlan_inn end --------------//

/* ===========================================================
 *        init_landColor_onlyHighLand
 * -----------------------------------------------------------
 * -- landColor 上色方案：
 *    让 density.lvl: 1，2，3 颜色逐渐加深。
 *    其余区域 保持原色 
 */
void EcoSysPlan::init_landColor_onlyHighLand( const RGBA &_baseColor ){

    this->landColors.resize( Density::get_idxNum() );
    //---
    for( int i=Density::get_minLvl(); i<=Density::get_maxLvl(); i++ ){ //- [-3,3]
        (i <= 0) ?
            this->landColors.at( Density::lvl_2_idx(i) ) = _baseColor :
            this->landColors.at( Density::lvl_2_idx(i) ) = _baseColor.add(  i * ecoSysPlan_inn::off_r, 
                                                                            i * ecoSysPlan_inn::off_g, 
                                                                            i * ecoSysPlan_inn::off_b, 
                                                                            0 );
    }
}


/* ===========================================================
 *        init_landColor_doubleDeep
 * -----------------------------------------------------------
 * -- landColor 上色方案：
 *    让 density.lvl: 1，  2， 3 颜色逐渐加深。
 *    让 density.lvl: -1，-2，-3 颜色逐渐加深。
 * 适合 密林，仅留下小路通行
 */
void EcoSysPlan::init_landColor_doubleDeep( const RGBA &_baseColor ){

    int absI {}; 
    this->landColors.resize( Density::get_idxNum() );
    //---
    for( int i=Density::get_minLvl(); i<=Density::get_maxLvl(); i++ ){ //- [-3,3]
        absI = std::abs(i);
        this->landColors.at( Density::lvl_2_idx(i) ) = _baseColor.add(  absI * ecoSysPlan_inn::off_r, 
                                                                        absI * ecoSysPlan_inn::off_g, 
                                                                        absI * ecoSysPlan_inn::off_b, 
                                                                        0 );
    }
}


/* ===========================================================
 *          init_landColor_twoPattern
 * -----------------------------------------------------------
 * -- landColor 上色方案：
 *    所有 密度大于等于 _density.lvl 的走 _color_1。
 *    剩下区域 走 _color_2
 */
void EcoSysPlan::init_landColor_twoPattern( const Density &_density_high,
                                    const RGBA &_color_high,
                                    const RGBA &_color_low,
                                    bool  is_goDeep_high,
                                    bool  is_goDeep_low ){

    int    absI {};
    this->landColors.resize( Density::get_idxNum() );
    //---
    for( int i=Density::get_minLvl(); i<=Density::get_maxLvl(); i++ ){ //- [-3,3]
        absI = std::abs(i);
        if( i >= _density_high.get_lvl() ){ //- high
            is_goDeep_high ?
                this->landColors.at( Density::lvl_2_idx(i) ) = _color_high.add( absI * ecoSysPlan_inn::off_r, 
                                                                                absI * ecoSysPlan_inn::off_g, 
                                                                                absI * ecoSysPlan_inn::off_b, 
                                                                                0 ) :
                this->landColors.at( Density::lvl_2_idx(i) ) = _color_high;
        }else{ //- low
            is_goDeep_low ?
                this->landColors.at( Density::lvl_2_idx(i) ) = _color_low.add(  absI * ecoSysPlan_inn::off_r, 
                                                                                absI * ecoSysPlan_inn::off_g, 
                                                                                absI * ecoSysPlan_inn::off_b, 
                                                                                0 ) :
                this->landColors.at( Density::lvl_2_idx(i) ) = _color_low;
        }
    }
}


/* ===========================================================
 *             init_densityDatas
 * -----------------------------------------------------------
 */
void EcoSysPlan::init_densityDatas( float _densitySeaLvlOff, 
                                const std::vector<float> &_datas ){

    tprAssert( !this->is_densityDivideVals_init );
    this->densitySeaLvlOff = _densitySeaLvlOff;

    //-- 确认个数不出错 --
    tprAssert( _datas.size() == Density::get_idxNum()-1 );
    //-- 确认 每个元素值 不越界 --
    for( const auto &i : _datas ){
        tprAssert( (i>=-100.0f) && (i<=100.0f) );
    }
    //-- 确认 每个元素值 递增 --
    float old {0.0f};
    float neo {0.0f};
    for( auto i=_datas.cbegin(); i!=_datas.cend(); i++ ){
        if( i == _datas.cbegin() ){
            old = *i;
        }else{
            neo = *i;
            tprAssert( old < neo );
            old = neo;
        }
    }
    //-- 正式搬运 --
    this->densityDivideVals.clear();
    this->densityDivideVals.insert( this->densityDivideVals.end(),
                                    _datas.begin(),
                                    _datas.end() );

    this->is_densityDivideVals_init = true;
}


/* ===========================================================
 *        init_goSpecIdPools_and_applyPercents
 * -----------------------------------------------------------
 */
void EcoSysPlan::init_goSpecIdPools_and_applyPercents(){
    tprAssert( (this->is_goSpecIdPools_init==false) && 
            (this->is_applyPercents_init==false) );
    this->goSpecIdPools.resize( Density::get_idxNum(), std::vector<goSpecId_t> {} );
    this->applyPercents.resize( Density::get_idxNum(), 0.0f );
    this->is_goSpecIdPools_init = true;
    this->is_applyPercents_init = true;
}


/* ===========================================================
 *              insert
 * -----------------------------------------------------------
 */
void EcoSysPlan::insert(const Density &_density, 
                    float _applyPercent,
                    const std::vector<EcoEnt> &_ecoEnts ){

    tprAssert( this->is_applyPercents_init ); //- MUST
    this->applyPercents.at(_density.get_idx()) = _applyPercent;

    goSpecId_t  id_l {};
    for( const auto &ent : _ecoEnts ){
        tprAssert( this->is_goSpecIdPools_init ); //- MUST
        auto &poolRef = this->goSpecIdPools.at(_density.get_idx());
        id_l = ssrc::get_goSpecId(ent.specName);
        poolRef.insert( poolRef.begin(), ent.idNum, id_l );
    }
}


/* ===========================================================
 *               shuffle_goSpecIdPools
 * -----------------------------------------------------------
 * -- 需要调用者 提供 seed
 *    通过这种方式，来实现真正的 伪随机
 */
void EcoSysPlan::shuffle_goSpecIdPools( u32_t _seed ){

    std::default_random_engine  rEngine; 
    rEngine.seed( _seed );
    for( auto &poolRef : this->goSpecIdPools ){
        std::shuffle( poolRef.begin(), poolRef.end(), rEngine );
    }
}


