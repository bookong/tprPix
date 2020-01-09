/*
 * =================== GoSpecFromJson.cpp ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.12.09
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "GoSpecFromJson.h"


//-------------------- Engine --------------------//
#include "GameObj.h"

#include "tprDebug.h"

//======== static ========//
std::unordered_map<goSpeciesId_t, std::unique_ptr<GoSpecFromJson>> GoSpecFromJson::dataUPtrs {};
std::unordered_map<goSpeciesId_t, std::string>     GoSpecFromJson::ids_2_names {};
std::unordered_map<std::string, goSpeciesId_t>     GoSpecFromJson::names_2_ids {};
std::unordered_map<goSpeciesId_t, F_GO_INIT>       GoSpecFromJson::initFuncs {}; 



void GoSpecFromJson::assemble_2_newGo( goSpeciesId_t specID_, GameObj &goRef_ ){

    const auto &d = GoSpecFromJson::get_goSpecFromJsonRef( specID_ );

    goRef_.speciesId = d.speciesId;
    goRef_.family    = d.family;
    goRef_.state     = d.state;
    goRef_.moveState = d.moveState;
    goRef_.move.set_MoveType( d.moveType );

    goRef_.isMoveCollide = d.isMoveCollide;

    auto &collisionRef = goRef_.get_collisionRef();

    collisionRef.set_isDoPass( d.isDoPass );
    collisionRef.set_isBePass( d.isBePass );
    if( !d.isBePass ){
        collisionRef.set_functor_Is_extraPassableDogoSpeciesId( 
            [&d](goSpeciesId_t dogoSpeciesId_)->bool {
                return (d.extraPassableDogoSpeciesIds.find(dogoSpeciesId_) != d.extraPassableDogoSpeciesIds.end());
            }
        );
    }

    goRef_.moveSpeedLvl.reset( d.moveSpeedLvl );
    goRef_.set_pos_alti( d.alti );
    goRef_.weight = d.weight;

    //------ pubBinary -------//
    goRef_.pubBinary.HP = d.pubBinary.HP;
    goRef_.pubBinary.MP = d.pubBinary.MP;

    //...
    //------ default -------//
    goRef_.isDirty = false;
    goRef_.isControlByPlayer = false;
    //...
    //------ tmp -------//
    goRef_.isActive = true; //- tmp. 是否进入激活圈，应该由 mpos 计算出来 未实现
    //...
}



void GoSpecFromJson::check_all_extraPassableDogoSpeciesIds()noexcept{

    for( const auto &ipair : GoSpecFromJson::dataUPtrs ){ // each goSpecFromJson
        GoSpecFromJson &goSpecFromJsonRef = *(ipair.second); 

        for( const auto &id : goSpecFromJsonRef.extraPassableDogoSpeciesIds ){
            tprAssert( GoSpecFromJson::ids_2_names.find(id) != GoSpecFromJson::ids_2_names.end() );
        }
    }
}



void GoSpecFromJson::init_check()noexcept{

    if( this->moveStateTableUPtr ){
        this->moveStateTableUPtr->init_check( this );
    }
}



void GoSpecFromJson::MoveStateTable::init_check( const GoSpecFromJson *goSpecFromJsonPtr_ )noexcept{

    //--1-- min <= max
    int minILvl = static_cast<int>( this->minLvl );
    int maxILvl = static_cast<int>( this->maxLvl );
    tprAssert( minILvl <= maxILvl );

    //--2-- min -> max 中间是连续的，每个元素都要存在
    for( int i=minILvl; i<=maxILvl; i++ ){
        SpeedLevel lvl = int_2_SpeedLevel( i );
        tprAssert( this->table.find(lvl) != this->table.end() );
    }
    
    for( const auto &ipair : this->baseSpeedLvls ){

        const std::string &actionName = ipair.first;
        const SpeedLevel &baseLvl = ipair.second;

        //--3-- 每个 action， afs.json 数据都要实现 
        // 暂未 实现 ...
        // ...

        //--4-- baseSpeedLvl 必须在 实际数据表中
        bool isFind { false };
        for( const auto &p : this->table ){
            if( (p.second==actionName) && (p.first==baseLvl) ){
                isFind = true;
                break;
            }
        }
        tprAssert( isFind );
    }
}




