/*
 * ======================= AnimAction.h =====================
 *                          -- tpr --
 *                                        CREATE -- 2019.05.06
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#ifndef TPR_ANIM_ACTION_H
#define TPR_ANIM_ACTION_H
//=== *** glad FIRST, glfw SECEND *** ===
#include<glad/glad.h>  
#include<GLFW/glfw3.h>

//-------------------- CPP --------------------//
#include <string>
#include <vector>
#include <functional>
#include <memory>

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "IntVec.h" 
#include "NineDirection.h"
#include "RGBA.h" 
#include "AnimActionPos.h"
#include "AnimLabel.h"
#include "functorTypes.h"


enum class AnimActionState{
    Stop,   // always for idle, and part of Once
    Working // always for Cycle, and part of Once
};

//-- 动作类型 --
enum class AnimActionType{
    Idle,   //- 从来不更换 动画帧
    Once,   //- 只播放一次，支持 逻辑节点帧，结束后自动跳转到预定的 新状态（未定）
    Cycle   //- 循环播放一段 动画帧
};

inline AnimActionType str_2_AnimActionType( const std::string &str_ )noexcept{
    if( str_ == std::string{"Idle"} ){
        return AnimActionType::Idle;
    }else if( str_ == std::string{"Once"} ){
        return AnimActionType::Once;
    }else if( str_ == std::string{"Cycle"} ){
        return AnimActionType::Cycle;
    }else{
        tprAssert(0);
        return AnimActionType::Idle; //- never reach
    }
}


//-- gomesh 自己保存的 有关 animAction 的动态数据。
class AnimActionPvtData{
public:
    size_t   currentIdx_for_frameIdxs {};   //- 当前真正播放的帧，在 frameIdxs 中的下标号
    //---
    size_t   currentFrameIdx {};  //- 当前指向的 frameidx 值
    size_t   currentTimeStep {};  //- 当前帧的 timeStep, （不应被外部访问）
                                  //  被 smoothDeltaTime 修正过
    //---
    size_t   updates {};     //- 切换一次帧后，记录 调用 update() 的次数
    //-- flags --//
    bool     isLastFrame {false}; //- 仅用于 Once 模式

    //--- functor ---//
    F_R_double  reset_playSpeedScale {nullptr};
};


//-- 3 kinds of AnimActionParam 
//   可能最终不会被使用 ...
enum class AnimActionParamType{
    singleFrame,
    singleFrame_batch, // 一组 单帧action，批处理
                        // 在未来，等树的动画完善后，这种 batch 类型会消失
    multiFrame_SameTimeStep,
    multiFrame_DiffTimeStep
};


//-- 作为 AnimFrameSet::insert_a_png() 参数 --
class AnimActionParam{
public:

    //-- 常规构造器,且手动设置 timesteps --
    AnimActionParam(size_t  subspeciesIdx_,
                    const std::string     &actionName_,
                    NineDirection          actionDir_,
                    AnimActionType         type_,
                    bool                   isOrder_,
                    bool                   isOpaque_,
                    size_t                 jFrameIdx_,
                    const std::vector<size_t>  &lFrameIdxs_,
                    const std::vector<size_t>  &timeSteps_,
                    const std::vector<AnimLabel> &labels_ ):
        subspeciesIdx(subspeciesIdx_),
        actionName(actionName_),
        actionDir(actionDir_),
        actionType( type_ ),
        isOrder( isOrder_ ),
        isOpaque( isOpaque_ ),
        isTimeStepsManualSet(true),
        jFrameIdx(jFrameIdx_),
        defaultTimeStep(6) //- 随便写一个值，反正用不上
        {
            this->lFrameIdxs.insert( this->lFrameIdxs.end(), lFrameIdxs_.begin(), lFrameIdxs_.end() );
            this->timeSteps.insert( this->timeSteps.end(), timeSteps_.begin(), timeSteps_.end() );
            this->animLabels.insert( this->animLabels.end(), labels_.begin(), labels_.end() );
        }

    //-- 常规构造器,但使用统一值的 timesteps --
    AnimActionParam(size_t  subspeciesIdx_,
                    const std::string    &actionName_,
                    NineDirection         actionDir_,
                    AnimActionType        type_,
                    bool                  isOrder_,
                    bool                  isOpaque_,
                    size_t                jFrameIdx_,
                    const std::vector<size_t>  &lFrameIdxs_,
                    size_t   _defaultTimeStep,
                    const std::vector<AnimLabel> &labels_ ):
        subspeciesIdx(subspeciesIdx_),
        actionName(actionName_),
        actionDir(actionDir_),
        actionType( type_ ),
        isOrder( isOrder_ ),
        isOpaque( isOpaque_ ),
        isTimeStepsManualSet(false),
        jFrameIdx(jFrameIdx_),
        defaultTimeStep(_defaultTimeStep)
        {
            this->lFrameIdxs.insert( this->lFrameIdxs.end(), lFrameIdxs_.begin(), lFrameIdxs_.end() );
            this->timeSteps.push_back( _defaultTimeStep ); //- 用不上
            this->animLabels.insert( this->animLabels.end(), labels_.begin(), labels_.end() );
        }

    //-- 单帧action 专用 构造器 --
    AnimActionParam(size_t  subspeciesIdx_,
                    const std::string &actionName_,
                    NineDirection      actionDir_,
                    size_t  jFrameIdx_,
                    size_t  lFrameIdx_,
                    bool    isOpaque_,
                    const std::vector<AnimLabel> &labels_ ):
        subspeciesIdx(subspeciesIdx_),
        actionName(actionName_),
        actionDir(actionDir_),
        actionType( AnimActionType::Idle ), //- 默认type
        isOrder( true ), //- 随便写一个值，反正用不上
        isOpaque( isOpaque_ ),
        isTimeStepsManualSet(false),
        jFrameIdx(jFrameIdx_),
        defaultTimeStep(6) //- 随便写一个值，反正用不上
        {
            this->lFrameIdxs.push_back( lFrameIdx_ );
            this->timeSteps.push_back( 6 ); //- 随便写一个值，反正用不上
            this->animLabels.insert( this->animLabels.end(), labels_.begin(), labels_.end() );
        }

    //===== vals =====//
    size_t          subspeciesIdx;
    std::string     actionName;
    NineDirection   actionDir;
    AnimActionType  actionType;
    bool            isOrder;
    bool            isOpaque;                //- 是否为 不透明图元
    bool            isTimeStepsManualSet;    //- 若为 false，参数 timeSteps_ 可为空容器
    size_t          jFrameIdx;               //- J帧序号，一个还在发展改善中的数值... 暂时手动设置
    size_t          defaultTimeStep;         //- 若上参数为 false，通过本参数来设置 timeSteps 
    std::vector<size_t> lFrameIdxs {};          //- 和 AnimAction 中的 frameIdxs 不同，此处基于的idx 是相对值
    std::vector<size_t> timeSteps  {}; 
    std::vector<AnimLabel> animLabels {};
};


//-- need --//
class AnimFrameSet;




//-- 本class 只存储 于 anim-action 有关的 所有静态数据
//   纯粹的静态数据，不允许存储任何与 gomesh相关的动态数据 ！！！！
//   动态数据 存储在 gomesh.animActionPvtData 中 （每一个 gomesh 独占一份）
class AnimAction{
    using F_UPDATE = std::function<void(AnimActionPvtData &)>;
public:
    AnimAction() = default;

    void init(  const AnimFrameSet &animFrameSetRef_,
                const AnimActionParam &param_,
                const AnimActionPos *animActionPosPtr_,
                IntVec2 pixNum_per_frame_,
                size_t headIdx_,
                bool isHaveShadow_ );

    F_UPDATE                update {nullptr};
    
    //- 当 gomesh 切换 animAction 时
    //  通过此函数，来重置自己的 pvtdata 值 --
    inline void reset_pvtData( AnimActionPvtData &pvtData_ )noexcept{
        pvtData_.currentIdx_for_frameIdxs = 0;
        pvtData_.currentFrameIdx = this->frameIdxs.at(0);
        pvtData_.currentTimeStep = this->timeSteps.at(0);
        pvtData_.isLastFrame = false;
    }
    
    //----- get -----//
    inline bool get_isHaveShadow() const noexcept{ return this->isHaveShadow; }
    inline bool get_isOpaque() const noexcept{ return this->isOpaque; }
    inline IntVec2 get_pixNum_per_frame() const noexcept{ return this->pixNum_per_frame; }
    inline AnimActionType get_animActionType()const noexcept{ return this->actionType; }
    
    inline const glm::dvec2 &get_currentRootAnchorDPosOff() const noexcept{
        return this->animActionPosPtr->get_rootAnchorDPosOff();
    }

    inline GLuint get_currentTexName_pic( const AnimActionPvtData &pvtData_ ) const noexcept{
        return this->texNames_pic_ptr->at(pvtData_.currentFrameIdx);
    }
    inline GLuint get_currentTexName_shadow( const AnimActionPvtData &pvtData_ ) const noexcept{
        return this->texNames_shadow_ptr->at(pvtData_.currentFrameIdx);
    }

    inline const AnimActionPos &get_currentAnimActionPos() const noexcept{
        return *this->animActionPosPtr;
    }


private:
    inline void update_idle( AnimActionPvtData &pvtData_ ){}
    void update_once( AnimActionPvtData &pvtData_ );
    void update_cycle( AnimActionPvtData &pvtData_ );

    size_t adjust_currentTimeStep( size_t currentTimeStep_, AnimActionPvtData &pvtData_ );

    //===== vals =====//
    //-- 从 animFrameSet 中获得的 只读指针 --
    const std::vector<GLuint> *texNames_pic_ptr    {nullptr};
    const std::vector<GLuint> *texNames_shadow_ptr {nullptr};

    const AnimActionPos *animActionPosPtr {nullptr}; // 1-animAction, 1-animActionPos 

    AnimActionType   actionType {}; 

    //- 用户可以手动编排 frameIdx 序列。同时，默认 容器中的第一帧，就是 enterIdx --
    std::vector<size_t> frameIdxs {}; //- 相对于 AnimFrameSet 全frames数据的 idx
    std::vector<size_t> timeSteps {};

    size_t  totalFrameNum {};   //- 本 action 有几帧

    IntVec2  pixNum_per_frame {};  //- 单帧画面 的 长宽 像素值

    //===== flags =====//
    bool   isHaveShadow {};
    bool   isOpaque     {};

};




#endif 
