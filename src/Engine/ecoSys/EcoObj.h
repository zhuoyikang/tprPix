/*
 * ========================== EcoObj.h =======================
 *                          -- tpr --
 *                                        CREATE -- 2019.03.02
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#ifndef TPR_ECO_OBJ_H
#define TPR_ECO_OBJ_H

//-------------------- CPP --------------------//
#include <vector>

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "tprCast.h"
#include "sectionKey.h"
#include "EcoSysPlanType.h"
#include "Quad.h"
#include "occupyWeight.h"
#include "RGBA.h"
#include "IntVec.h"
#include "GameObjType.h"
#include "Density.h"
#include "GoSpecData.h"
#include "colorTableId.h"

#include "DensityPool.h"

class EcoSysPlan;


//- 一个在游戏地图上存在的 实实在在的区域。
//- 在游戏世界中，每个 section左下角，都放置一个 EcoSysPlan 数据集 
//  存储了这个 EcoSysPlan 实例 的各种数据 （用来生成地图）
//  ----
//  这组数据会暂时 长期存储在 mem/disk
class EcoObj{
public:
    EcoObj() = default;

    void init_for_node( sectionKey_t sectionKey_ );
    void init_for_regular(  sectionKey_t sectionKey_,
                            const std::vector<sectionKey_t> &nearby_four_ecoSysPlanIds_ );

    void init_fstOrder( sectionKey_t sectionKey_ );


    const DensityPool &get_densityPool( size_t densityIdx_ )const noexcept{
        /*
        tprAssert( densityIdx_ < this->densityPools.size() ); // tmp
        return *(this->densityPools.at(densityIdx_));
        */

        tprAssert( densityIdx_ < this->densityPoolsPtr->size() ); // tmp
        return *(this->densityPoolsPtr->at(densityIdx_));
    }


    inline IntVec2          get_mpos() const noexcept{ return this->mcpos.get_mpos(); }
    inline ecoSysPlanId_t   get_ecoSysPlanId() const noexcept{ return this->ecoSysPlanId; }
    inline EcoSysPlanType   get_ecoSysPlanType() const noexcept{ return this->ecoSysPlanType; }
    inline double           get_densitySeaLvlOff() const noexcept{ return this->densitySeaLvlOff; }
    inline sectionKey_t     get_sectionKey() const noexcept{ return this->sectionKey; }
    inline size_t           get_uWeight() const noexcept{ return this->uWeight; }
    inline occupyWeight_t   get_occupyWeight() const noexcept{ return this->occupyWeight; }
    inline colorTableId_t   get_colorTableId()const noexcept{ return this->colorTableId; }
    inline const std::vector<double> *get_densityDivideValsPtr() const noexcept{ return this->densityDivideValsPtr; }

    //======== static funcs ========// 
    static void calc_nearFour_node_ecoObjKey(  sectionKey_t targetKey_, 
                                                    std::vector<sectionKey_t> &container_ );
               
private:
    void copy_datas_from_ecoSysPlan( EcoSysPlan *targetEcoPtr_ );

    //--- 只能被 非node 实例调用 --
    void init_for_no_node_ecoObj( const std::vector<sectionKey_t> &nearby_four_ecoSysPlanIds_ );
    
    //======== vals ========//
    sectionKey_t  sectionKey {};
    MapCoord      mcpos  {}; //- [left-bottom]
                      
    size_t uWeight {}; // [0, 9999]

    occupyWeight_t  occupyWeight {0}; //- 抢占权重。 [0,15]
                            //- 数值越高，此 ecosys 越强势，能占据更多fields
                            //- [just mem] 

    ecoSysPlanId_t  ecoSysPlanId {};
    EcoSysPlanType  ecoSysPlanType  {EcoSysPlanType::Forest};

    colorTableId_t  colorTableId {}; // same with ecoPlan.colorTableId

    //-- 本 ecoObj mpos 在 世界坐标中的 奇偶性 --
    // 得到的值将会是 {0,0}; {1,0}; {0,1}; {1,1} 中的一种
    IntVec2  oddEven {}; 
    
    //-- field.nodeAlit.val > 30;
    //-- field.density.lvl [-3, 3] 共 7个池子
    //-- 用 density.get_idx() 来遍历
    //  实际数据 存储在 ecosysPlan 实例中，此处仅保存 只读指针 --
    //const std::vector<double> *applyPercentsPtr {}; //- each entry: [0.0, 1.0]

                    //- 已被放入 densitypool ...


    const std::vector<double> *densityDivideValsPtr {};  //- 6 ents, each_ent: [-100.0, 100.0]
                        

    //-- 独立数据 --
    //std::vector<std::unique_ptr<DensityPool>> densityPools {};
    const std::vector<std::unique_ptr<DensityPool>> *densityPoolsPtr {nullptr};

                            // 暂时没有确定，是否重分配 densitypools 数据
                            // ...



    double           densitySeaLvlOff  {0.0}; 
};


#endif 
