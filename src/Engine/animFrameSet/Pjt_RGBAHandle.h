/*
 * ========================= Pjt_RGBAHandle.h ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.08.31
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 *    在 "dog.J.png" 中，记载了每个图元帧的 pjt信息
 *    本模块就是用来 解析读取这些信息，将它们还原为 外部所需的状态
 * ----------------------------
 */
#ifndef TPR_PJT_RGBA_HANDLE_2_H
#define TPR_PJT_RGBA_HANDLE_2_H
//------------------- C --------------------//
#include <cmath>

//------------------- Libs --------------------//
#include "tprDataType.h"

//------------------- Engine --------------------//
#include "tprAssert.h"
#include "RGBA.h"
#include "GoAltiRange.h" 
#include "AnimActionPos.h"
#include "ColliderType.h"


#include "tprDebug.h"


namespace pjt_RGBAHandle_2_inn{//---------- namespace ---------//
    //--- A --- 
    u8_t    A_OPAQUE         { 255 }; // alpha
    //--- R --- 
    u8_t    R_rootAnchor    { 255 };
    u8_t    R_tailAnchor     { 155 };//- 仅用于 胶囊体，副端点

    //--- G --- 
    u8_t  G_moveColliRadius   { 150 };
    u8_t  G_skillColliRadius  { 250 }; //- 胶囊体中，没有此点，此值将被同步为 moveColliRadius 

    //--- B --- 
    
    RGBA  uselessColor_1  { 150, 150, 150, 255 };

    RGBA  uselessColor_2  { 0, 0, 150, 255 }; // 蓝幕底色，直接忽略

}//------------ namespace: end ---------//

//-- 只在 animFrameSet 资源被加载时 才被调用，对性能无要求 --
//  use:
//  - Pjt_RGBAHandle jh { 5 };
//  - jh.set_rgba( _pixColor );
//  - XXX = jh.is_colliEntHead();
class Pjt_RGBAHandle{
public:
    explicit Pjt_RGBAHandle( int off_=5 ):
        off(off_)
        {}    

    //-- 忠实地记录从 Jpng 中读取地每像素信息 --
    // 而不关心实际 ColliderType，这部分由 AnimActionSemiData 去完成
    inline void set_rgba(   AnimActionSemiData *semiDataPtr_, 
                            const RGBA &rgba_, 
                            const glm::dvec2 &pixDPos_ 
                            )noexcept{
        //-- reset --
        this->rgba = rgba_;
        this->animActionSemiDataPtr = semiDataPtr_;

        //=== A ===
        //--- ignore ---
        if( (is_near_inner(RGBA_ChannelType::A, pjt_RGBAHandle_2_inn::A_OPAQUE)==false) ||
            (this->rgba.is_near( pjt_RGBAHandle_2_inn::uselessColor_1, 5)==true) ||
            (this->rgba.is_near( pjt_RGBAHandle_2_inn::uselessColor_2, 5)==true) ){
            return;
        }

        //=== R ===
        //--- rootAnchor ---
        if( is_near_inner( RGBA_ChannelType::R, pjt_RGBAHandle_2_inn::R_rootAnchor ) ){
            this->set_goAltiRange();
            this->animActionSemiDataPtr->set_rootAnchor_onlyOnce( pixDPos_ );
            return;
        }
        //--- isTailAnchor ---
        if( is_near_inner( RGBA_ChannelType::R, pjt_RGBAHandle_2_inn::R_tailAnchor ) ){
            this->animActionSemiDataPtr->set_tailAnchor_onlyOnce( pixDPos_ );
            return;
        }
            
        //=== G ===
        //--- moveColliRadius ---
        if( is_near_inner( RGBA_ChannelType::G, pjt_RGBAHandle_2_inn::G_moveColliRadius ) ){
            this->animActionSemiDataPtr->set_moveColliRadiusAnchor_onlyOnce( pixDPos_ );
            return;
        }
        //--- skillColliRadius ---
        if( is_near_inner( RGBA_ChannelType::G, pjt_RGBAHandle_2_inn::G_skillColliRadius ) ){
            this->animActionSemiDataPtr->set_skillColliRadiusAnchor_onlyOnce( pixDPos_ );
            return;
        }
            
        //=== B === 
        //...more...

        //-- 遇到了 未识别的 颜色 --
                cout << "Pjt_RGBAHandle: Unexpected Color: " 
                    << static_cast<int>(this->rgba.r) << ", "
                    << static_cast<int>(this->rgba.g) << ", "
                    << static_cast<int>(this->rgba.b) << ", "
                    << static_cast<int>(this->rgba.a) << "; "
                    << endl;
        tprAssert(0);
    }

private:

    inline bool is_near_inner( RGBA_ChannelType ct_, u8_t target_ )noexcept{
        switch( ct_ ){
            case RGBA_ChannelType::R:  return (abs(static_cast<int>(this->rgba.r-target_)) <= this->off);
            case RGBA_ChannelType::G:  return (abs(static_cast<int>(this->rgba.g-target_)) <= this->off);
            case RGBA_ChannelType::B:  return (abs(static_cast<int>(this->rgba.b-target_)) <= this->off);
            case RGBA_ChannelType::A:  return (abs(static_cast<int>(this->rgba.a-target_)) <= this->off);
            default:
                tprAssert(0);
                return  false; //- never touch -
        }
    } 

    //-- 将 rgba 态 高度信息，转换为 mem态 goAltiRange值 --
    //
    //     在 8m8 改为 64m64 后，原有的高度数值不管用了。
    //     目前的临时记录法是：实际像素高度／10。
    //     可以预测，这个方法 可能会在 引入跳跃系统时 带来麻烦
    //
    inline void set_goAltiRange()noexcept{
        u8_t low  = this->rgba.g;
        u8_t high = this->rgba.b;
        tprAssert( low < high );
        this->animActionSemiDataPtr->set_lGoAltiRange_onlyOnce( static_cast<char>(low), static_cast<char>(high) );
    }

    //-- 检测 参数 _beCheck，是否在 [_low,_low+_off) 区间内
    inline bool is_in_range( u8_t beCheck_, u8_t low_, u8_t off_ )noexcept{
        return ((beCheck_>=low_) && (beCheck_<(low_+off_)));
    }

    //======== vals ========//
    RGBA                 rgba         {}; //- 本模块处理的数据
    AnimActionSemiData  *animActionSemiDataPtr {nullptr};
    int                  off          {}; //- 颜色误差. 为了照顾 std::abs(), 改用 int 类型
};

#endif 

