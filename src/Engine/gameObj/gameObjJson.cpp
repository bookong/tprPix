/*
 * ================= gameObjJson.cpp =====================
 *                          -- tpr --
 *                                        CREATE -- 2019.07.02
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
//--------------- CPP ------------------//
#include <unordered_map>
#include <string>
#include <utility>

//--------------- Libs ------------------//
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "tprGeneral.h"

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "global.h"
#include "fileIO.h"
#include "GameObj.h"
#include "AnimFrameSet.h"

#include "GoSpecFromJson.h"

#include "json_oth.h"

#include "esrc_state.h"


using namespace rapidjson;

#include <iostream>
using std::cout;
using std::endl;

namespace json{//------------- namespace json ----------------
namespace goJson_inn {//-------- namespace: goJson_inn --------------//

    void parse_single_jsonFile( const std::string &path_file_ );

}//------------- namespace: goJson_inn end --------------//



/* Do Not Worry About Performance !!!
 */
void parse_goJsonFile(){

    cout << "   ----- parse_goJsonFile: start ----- " << endl;


    std::vector<std::string> path_files {};
    collect_fileNames( path_gameObjDatas, "", "_files.json", path_files );

    for( const auto &i : path_files ){
        goJson_inn::parse_single_jsonFile(i);
    }

    //--
    GoSpecFromJson::check_all_extraPassableDogoSpeciesIds();

    esrc::insertState("json_gameObj");
    cout << "   ----- parse_goJsonFile: end ----- " << endl;
}


namespace goJson_inn {//-------- namespace: goJson_inn --------------//



void parse_moveStateTable( const Value &pngEnt_, GoSpecFromJson &goSpecFromJsonRef_ );



void parse_single_jsonFile( const std::string &path_file_ ){

    //-----------------------------//
    //         load file
    //-----------------------------//
    auto jsonBufUPtr = read_a_file( path_file_ );

    //-----------------------------//
    //      parce JSON data
    //-----------------------------//
    Document doc;
    doc.Parse( jsonBufUPtr->c_str() );

    std::string goSpeciesName {};

    tprAssert( doc.IsArray() );
    for( auto &ent : doc.GetArray() ){

        {//--- goSpeciesName ---//
            const auto &a = check_and_get_value( ent, "goSpeciesName", JsonValType::String );
            goSpeciesName = a.GetString();
        }

        auto &goSpecFromJsonRef = GoSpecFromJson::create_new_goSpecFromJson( goSpeciesName );

        {//--- family ---//
            const auto &a = check_and_get_value( ent, "family", JsonValType::String );
            goSpecFromJsonRef.family = str_2_GameObjFamily( a.GetString() );
        }
        {//--- state ---//
            const auto &a = check_and_get_value( ent, "state", JsonValType::String );
            goSpecFromJsonRef.state = str_2_GameObjState( a.GetString() );
        }
        {//--- moveState ---//
            const auto &a = check_and_get_value( ent, "moveState", JsonValType::String );
            goSpecFromJsonRef.moveState = str_2_GameObjMoveState( a.GetString() );
        }
        {//--- moveType ---//
            const auto &a = check_and_get_value( ent, "moveType", JsonValType::String );
            goSpecFromJsonRef.moveType = str_2_MoveType( a.GetString() );
        }
        {//--- isMoveCollide ---//
            const auto &a = check_and_get_value( ent, "isMoveCollide", JsonValType::Bool );
            goSpecFromJsonRef.isMoveCollide = a.GetBool();
        }
        {//--- isDoPass ---//
            const auto &a = check_and_get_value( ent, "isDoPass", JsonValType::Bool );
            goSpecFromJsonRef.isDoPass = a.GetBool();
        }
        {//--- isBePass ---//
            const auto &a = check_and_get_value( ent, "isBePass", JsonValType::Bool );
            goSpecFromJsonRef.isBePass = a.GetBool();
        }

        //--- extraPassableDogoSpeciesNames ---//
        if( (!goSpecFromJsonRef.isBePass) &&
            ent.HasMember("extraPassableDogoSpeciesNames") ){

            const auto &names = check_and_get_value( ent, "extraPassableDogoSpeciesNames", JsonValType::Array );
            for( const auto &name : names.GetArray() ){
                tprAssert( name.IsString() );
                goSpecFromJsonRef.insert_2_ExtraPassableDogoSpeciesIds( name.GetString() );
            }
        }

        {//--- moveSpeedLvl ---//
            const auto &a = check_and_get_value( ent, "moveSpeedLvl", JsonValType::Int );
            goSpecFromJsonRef.moveSpeedLvl = int_2_SpeedLevel( a.GetInt() );
        }


        //--- moveStateTable ---//
        if( ent.HasMember("moveStateTable") ){
            parse_moveStateTable( ent, goSpecFromJsonRef );
        }
        

        {//--- alti ---//
            const auto &a = check_and_get_value( ent, "alti", JsonValType::Number );
            goSpecFromJsonRef.alti = get_double( a );
        }
        {//--- weight ---//
            const auto &a = check_and_get_value( ent, "weight", JsonValType::Number );
            goSpecFromJsonRef.weight = get_double( a );
        }
        {//--- pub.HP ---//
            tprAssert( ent.HasMember("pub.HP") );
            const Value &a = ent["pub.HP"];
            std::pair<bool,int> pair = get_nullable_int( a );
            goSpecFromJsonRef.pubBinary.HP = (pair.first) ? pair.second : -999; //- tmp
        }
        {//--- pub.MP ---//
            tprAssert( ent.HasMember("pub.MP") );
            const Value &a = ent["pub.MP"];
            std::pair<bool,int> pair = get_nullable_int( a );
            goSpecFromJsonRef.pubBinary.MP = (pair.first) ? pair.second : -999; //- tmp
        }

        //====================================//
        //    afs.json / multiGoMesh.json
        //------------------------------------//
        // xx.go.json 文件 所在 目录 的 path
        std::string dirPath = tprGeneral::get_dirPath( path_file_ );

        std::string lPath {};
        
        {//--- afs_Paths ---//
            std::string afs_Path {};
            const auto &afs_lPaths = check_and_get_value( ent, "afs_lPaths", JsonValType::Array );
            tprAssert( afs_lPaths.Size() != 0 ); // Must Have Ents !!!
            for( const auto &i : afs_lPaths.GetArray() ){
                tprAssert( i.IsString() );
                lPath = i.GetString();
                tprAssert( lPath != "" ); // MUST EXIST !!!
                afs_Path = tprGeneral::path_combine( dirPath, lPath ); // 绝对路径名
                json::parse_single_animFrameSetJsonFile(afs_Path, 
                                                        goSpecFromJsonRef.get_afsNamesPtr() );
                                                        // 目前，实际存储地 还是在 esrc 中
            }
        }

        {//--- multiGoMesh_Paths ---//
            std::string multiGoMesh_Path {};
            const auto &multiGoMesh_lPaths = check_and_get_value( ent, "multiGoMesh_lPaths", JsonValType::Array );
            for( const auto &i : multiGoMesh_lPaths.GetArray() ){
                tprAssert( i.IsString() );
                lPath = i.GetString();
                tprAssert( lPath != "" ); // MUST EXIST !!!
                multiGoMesh_Path = tprGeneral::path_combine( dirPath, lPath ); // 绝对路径名
                json::parse_single_multiGoMeshJsonFile( multiGoMesh_Path );
                            // 数据直接存储在 GoSpecFromJson 中
            }
        }

        //====================================//
        //     init check
        //------------------------------------//
        goSpecFromJsonRef.init_check();
        
    }
}




void parse_moveStateTable( const Value &pngEnt_, GoSpecFromJson &goSpecFromJsonRef_ ){

    const auto &moveStateTable = check_and_get_value( pngEnt_, "moveStateTable", JsonValType::Object );

    goSpecFromJsonRef_.moveStateTableUPtr = std::make_unique<GoSpecFromJson::MoveStateTable>();
    GoSpecFromJson::MoveStateTable &tRef = *(goSpecFromJsonRef_.moveStateTableUPtr);

    {//--- minSpeedLvl ---//
        const auto &a = check_and_get_value( moveStateTable, "minSpeedLvl", JsonValType::Int );
        tRef.minLvl = int_2_SpeedLevel( a.GetInt() );
    }
    {//--- maxSpeedLvl ---//
        const auto &a = check_and_get_value( moveStateTable, "maxSpeedLvl", JsonValType::Int );
        tRef.maxLvl = int_2_SpeedLevel( a.GetInt() );
    }

    //--- table ---//
    std::string actionName {};
    SpeedLevel  baseSpeedLvl {};
    std::vector<SpeedLevel> speedLvls {};

    const auto &table = check_and_get_value( moveStateTable, "table", JsonValType::Array );
    for( const auto &tableEnt : table.GetArray() ){
        tprAssert( tableEnt.IsObject() );

        speedLvls.clear();

        {//--- actionName ---//
            const auto &a = check_and_get_value( tableEnt, "actionName", JsonValType::String );
            actionName = a.GetString();
        }
        {//--- baseSpeedLvl ---//
            const auto &a = check_and_get_value( tableEnt, "baseSpeedLvl", JsonValType::Int );
            baseSpeedLvl = int_2_SpeedLevel( a.GetInt() );
        }
        {//--- speedLvls ---//
            const auto &lvls = check_and_get_value( tableEnt, "speedLvls", JsonValType::Array );
            for( const auto &lvl : lvls.GetArray() ){
                tprAssert( lvl.IsInt() );
                speedLvls.push_back( int_2_SpeedLevel(lvl.GetInt()) );
            }
        }

        //=== 将临时数据 合成进 moveStateTable 实例 中 ===
        auto outPair1 = tRef.baseSpeedLvls.insert({ actionName, baseSpeedLvl });
        tprAssert( outPair1.second );
        for( const auto &lvl : speedLvls ){
            auto outPair2 = tRef.table.insert({ lvl, actionName });
        }
    }
}




}//------------- namespace: goJson_inn end --------------//
}//------------- namespace json: end ----------------

