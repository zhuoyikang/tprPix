/*
 * ========================= OneEyeBoy.cpp ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.01.30
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "Script/gameObjs/majorGos/OneEyeBoy.h"

//-------------------- CPP --------------------//
#include <functional>
#include <string>

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "Density.h"
#include "animSubspeciesId.h"

#include "esrc_shader.h" 
#include "esrc_gameSeed.h"
#include "esrc_animFrameSet.h"

//-------------------- Script --------------------//
#include "Script/resource/ssrc_all.h" 

using namespace std::placeholders;

#include "tprDebug.h" 


namespace gameObjs {//------------- namespace gameObjs ----------------



namespace oneEyeBoy_inn {//----------- namespace: oneEyeBoy_inn ----------------//

    inline std::uniform_int_distribution<int>     uDistribution_int(      3,   50 );
    inline std::uniform_real_distribution<double> uDistribution_double( -1.3, 1.3 );

}//-------------- namespace: oneEyeBoy_inn end ----------------//

struct OneEyeBoy_PvtBinary{
    animSubspeciesId_t subspeciesId {};
    int        tmp {};
    int        timeStep  {10};
    int        timeCount {};
    glm::dvec2 moveVec {}; //- 位移向量，确保是 标准向量
};


/* ===========================================================
 *                   init
 * -----------------------------------------------------------
 */
void OneEyeBoy::init(GameObj &goRef_, const DyParam &dyParams_ ){

    //================ go.pvtBinary =================//
    auto *pvtBp = goRef_.init_pvtBinary<OneEyeBoy_PvtBinary>();
    
    //================ dyParams =================//
    size_t randUVal {};
    const DyParams_Field *msParamPtr {nullptr};
    //---    
    size_t typeHash = dyParams_.get_typeHash();
    if( dyParams_.is_Nil() ){
        randUVal = 17; //- 随便写

        pvtBp->subspeciesId = esrc::apply_a_random_animSubspeciesId( "simpleMan", emptyAnimLabels, 10 ); //- 暂时只有一个 亚种

    }else if( typeHash == typeid(DyParams_Field).hash_code() ){
        msParamPtr = dyParams_.get_binaryPtr<DyParams_Field>();

        randUVal = 33; //- 随便写

        const auto &job_goMeshs = *(msParamPtr->job_goMeshsPtr);
        tprAssert( job_goMeshs.size() == 1 );
        pvtBp->subspeciesId = job_goMeshs.begin()->subspecId;

    }else{
        tprAssert(0); //- 尚未实现
    }

    

    


    //pvtBp->subspeciesId = esrc::apply_a_random_animSubspeciesId( "simpleMan", emptyAnimLabels, 10 ); //- 暂时只有一个 亚种


    //----- must before creat_new_goMesh() !!! -----//
    goRef_.set_actionDirection( apply_a_random_direction_without_mid(randUVal) );

    //================ animFrameSet／animFrameIdxHandle/ goMesh =================//
        //-- 制作唯一的 mesh 实例: "root" --
        goRef_.creat_new_goMesh("root", //- gmesh-name
                                pvtBp->subspeciesId,
                                "idle",
                                RenderLayerType::MajorGoes, //- 不设置 固定zOff值
                                &esrc::get_shaderRef(ShaderType::UnifiedColor),  // pic shader
                                glm::vec2{ 0.0f, 0.0f }, //- pposoff
                                0.0,  //- off_z
                                true //- isVisible
                                );
        
    //================ bind callback funcs =================//
    //-- 故意将 首参数this 绑定到 保留类实例 dog_a 身上
    goRef_.RenderUpdate = std::bind( &OneEyeBoy::OnRenderUpdate,  _1 );   
    goRef_.LogicUpdate  = std::bind( &OneEyeBoy::OnLogicUpdate,   _1 );

    //-------- actionSwitch ---------//
    goRef_.actionSwitch.bind_func( std::bind( &OneEyeBoy::OnActionSwitch,  _1, _2 ) );
    goRef_.actionSwitch.signUp( ActionSwitchType::Idle );
    goRef_.actionSwitch.signUp( ActionSwitchType::Move );


    //================ go self vals =================//   

}

/* ===========================================================
 *                       bind
 * -----------------------------------------------------------
 * -- 在 “工厂”模式中，将本具象go实例，与 一个已经存在的 go实例 绑定。
 * -- 这个 go实例 的类型，应该和 本类一致。
 */
void OneEyeBoy::bind( GameObj &goRef_ ){
}


/* ===========================================================
 *                       rebind
 * -----------------------------------------------------------
 * -- 从硬盘读取到 go实例数据后，重bind callback
 * -- 会被 脚本层的一个 巨型分配函数 调用
 */
void OneEyeBoy::rebind( GameObj &goRef_ ){
}

/* ===========================================================
 *                      OnRenderUpdate
 * -----------------------------------------------------------
 */
void OneEyeBoy::OnRenderUpdate( GameObj &goRef_ ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    auto *pvtBp = goRef_.get_pvtBinaryPtr<OneEyeBoy_PvtBinary>();

    //=====================================//
    //            AI
    //-------------------------------------//
    //...

    
    if( !goRef_.isControlByPlayer ){
        //-- 简单的随机游走
        pvtBp->timeCount++;


        if( pvtBp->timeStep == 0 ){
            pvtBp->timeStep = 13;
        }

        if( (pvtBp->timeCount % pvtBp->timeStep) == 0 ){

            auto &engine = esrc::get_gameSeed().getnc_realRandEngine();

            pvtBp->timeStep = oneEyeBoy_inn::uDistribution_int(engine) + 1; //- 每帧都更新 间隔值

            glm::dvec2 randVec {oneEyeBoy_inn::uDistribution_double(engine),
                                oneEyeBoy_inn::uDistribution_double(engine) };

            pvtBp->moveVec = glm::normalize( pvtBp->moveVec + randVec );
            
        }

        //-- 确保每一帧都位移，但只在一段时间后 才修改 位移方向 --
        goRef_.move.set_newCrawlDirAxes( DirAxes{pvtBp->moveVec} );
    }
    
    
    



    //=====================================//
    //         更新 位移系统
    //-------------------------------------//
    goRef_.move.RenderUpdate();

    //=====================================//
    //  将 确认要渲染的 goMeshs，添加到 renderPool         
    //-------------------------------------//
    goRef_.render_all_goMesh();
}


/* ===========================================================
 *                        OnLogicUpdate
 * -----------------------------------------------------------
 */
void OneEyeBoy::OnLogicUpdate( GameObj &goRef_ ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    auto *pvtBp = goRef_.get_pvtBinaryPtr<OneEyeBoy_PvtBinary>();
    //=====================================//

    // 什么也没做...
}


/* ===========================================================
 *               OnActionSwitch
 * -----------------------------------------------------------
 * -- 
 */
void OneEyeBoy::OnActionSwitch( GameObj &goRef_, ActionSwitchType type_ ){


        // 不应该直接是 input 中的 dir，而要基于 go.dir 来修正


        //cout << "OneEyeBoy::OnActionSwitch" << endl;
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    auto *pvtBp = goRef_.get_pvtBinaryPtr<OneEyeBoy_PvtBinary>();
    //=====================================//

    //-- 获得所有 goMesh 的访问权 --
    GameObjMesh &goMeshRef = goRef_.get_goMeshRef("root");

    //-- 处理不同的 actionSwitch 分支 --
    switch( type_ ){
        case ActionSwitchType::Idle:
            goMeshRef.bind_animAction( pvtBp->subspeciesId, goRef_.get_actionDirection(), "idle" );
            goRef_.rebind_rootAnimActionPosPtr(); //- 临时性的方案 ...
            break;

        case ActionSwitchType::Move:
            goMeshRef.bind_animAction( pvtBp->subspeciesId, goRef_.get_actionDirection(), "walk" );
            goRef_.rebind_rootAnimActionPosPtr(); //- 临时性的方案 ...
            break;

        default:
            break;
            //-- 并不报错，什么也不做...

    }

    


}


}//------------- namespace gameObjs: end ----------------
