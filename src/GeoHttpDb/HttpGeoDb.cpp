#include "HttpGeoDb.h"
#include "../db/BackDb.h"
#include <json/json.h>
#include <sys/param.h>

FileTask* GeoHttpTask::NewGeoHttpTask ( int fd,CHttpHost* host )
{
    return new GeoHttpTask ( fd,host );
}

GeoHttpTask::GeoHttpTask ( int fd,CHttpHost* host ) :
    HttpTask ( fd,host )
{

}

void GeoHttpTask::ProcessRequest()
{
    UriSplit urisplit;
    CHttpServerResponse response;
    response.Vision ( request.Vision() );
    if ( false == urisplit.Decode ( request.Uri().c_str() ) ) {
        response.Message ( 404 );
        return;
    }

    if ( urisplit.GetPath().size() < 2 ) {
        response.Message ( 404 );
        return;
    }

    std::string content;
    response.Message ( 200 );
    if ( strcasecmp ( urisplit.GetPath() [0].c_str(), "geo" ) == 0 ) {
        this->ProcessGeoPoint ( urisplit, response, content );
    }
    if ( content.size() > 0 ) {
        response.setContentLength ( content.size() );
        std::string maintype ( "text" );
        std::string subtype ( "plain" );
        response.setContentType ( maintype, subtype );
    }

    std::string reshead = response.SaveHead();
    WriteOut ( reshead.c_str(), reshead.size() );
    if ( content.size() > 0 ) {
        WriteOut ( content.c_str(), content.size() );
    }
}
void GeoHttpTask::ProcessGeoPoint ( UriSplit& urisplit,
                                    CHttpServerResponse& response, std::string& rescontent )
{
    if ( strcasecmp ( "save", urisplit.GetPath() [1].c_str() ) == 0 ) {
        if ( urisplit.ParamExist ( "id" ) && urisplit.ParamExist ( "lat" )
                && urisplit.ParamExist ( "lng" ) ) {
            int id = atoi ( urisplit.GetParam ( "id" ).c_str() );
            float lat = atof ( urisplit.GetParam ( "lat" ).c_str() );
            float lng = atof ( urisplit.GetParam ( "lng" ).c_str() );
            CBackDb::Instanse().AddPoint ( id, lat, lng );
            response.Message ( 200 );
            rescontent = "insert ok";
        } else {
            response.Message ( 500 );
            rescontent = "not enough param";
        }
    } else if ( strcasecmp ( "area", urisplit.GetPath() [1].c_str() ) == 0 ) {
        float lat1 = atof ( urisplit.GetParam ( "lat1" ).c_str() );
        float lng1 = atof ( urisplit.GetParam ( "lng1" ).c_str() );
        float lat2 = atof ( urisplit.GetParam ( "lat2" ).c_str() );
        float lng2 = atof ( urisplit.GetParam ( "lng2" ).c_str() );
        std::vector<GeoPoint> list;

        CBackDb::Instanse().ReadAreaPoint ( MIN ( lat1,lat2 ), MAX ( lat1,lat2 ),
                                            MIN ( lng1,lng2 ), MAX ( lng1,lng2 ), list );

        json_object* root = json_object_new_object();
        json_object* data = json_object_new_array();
        json_object_object_add ( root, "data", data );
        for ( std::vector<GeoPoint>::iterator i = list.begin();
                i != list.end(); i++ ) {
            json_object* ptvalue = json_object_new_object();
            json_object_array_add ( data, ptvalue );

            json_object* tempvalue;
            tempvalue = json_object_new_int ( i->id );
            json_object_object_add ( ptvalue, "id", tempvalue );
            tempvalue = json_object_new_double ( i->lat );
            json_object_object_add ( ptvalue, "lat", tempvalue );
            tempvalue = json_object_new_double ( i->lng );
            json_object_object_add ( ptvalue, "lng", tempvalue );
        }
        rescontent = json_object_to_json_string_ext ( root,JSON_C_TO_STRING_PLAIN );
        json_object_put ( root );

    } else {
        response.Message ( 404 );
    }
}
