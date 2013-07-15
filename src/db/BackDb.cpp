#include "BackDb.h"
#include <sys/stat.h>
#include <arpa/inet.h>
#include "../config/ConfigReader.h"
#include "../tools/gtools.h"
#include "geofunction.h"
#include <boost/regex.hpp>
#include <string.h>
void CBackDb::db_errcall_fcn ( const DB_ENV* dbenv, const char* errpfx,
                               const char* msg )
{
    printf ( "%s\n", msg );
}
CBackDb& CBackDb::Instanse()
{
    static CBackDb backdb;
    return backdb;
}
int get_point_list ( DB* sdbp, // secondary db handle
                     const DBT* pkey, // primary db record's key
                     const DBT* pdata, // primary db record's data
                     DBT* skey )
{
    if ( pdata->size == 0 ) {
        return DB_DONOTINDEX;
    }
    /*
    DbValue::PointList ptlist;
    if (false == ptlist.ParseFromArray(pdata->data, pdata->size))
        return DB_DONOTINDEX;
    DBT* pointlist = (DBT*) malloc(sizeof(DBT) * ptlist.points_size());
    memset(pointlist, 0, sizeof(DBT) * ptlist.points_size());
    skey->data = pointlist;
    skey->size = ptlist.points_size();
    skey->flags = DB_DBT_MULTIPLE | DB_DBT_APPMALLOC;

    for (int i = 0; i < ptlist.points_size(); i++) {
        int id = ptlist.points(i).id();
        int *idtmp = (int*) malloc(sizeof(int));
        *idtmp = htonl(id);
        DBT* dbt = pointlist + i;
        dbt->data = idtmp;
        dbt->size = sizeof(int);
        dbt->flags = DB_DBT_APPMALLOC;
    }*/
    skey->data=malloc ( pdata->size );
    skey->size=pdata->size;
    skey->flags=DB_DBT_APPMALLOC;
    memcpy ( skey->data,pdata->data,pdata->size );
    return 0;
}
CBackDb::CBackDb() :
    myEnv ( 0 ), canwrite ( false )
{

    ConfigReader config;
    config.LoadFile ( SelfPath() + "repdb.txt" );

    const static u_int32_t env_flags = DB_CREATE | DB_THREAD | DB_RECOVER
                                       | DB_LOCKDOWN | DB_PRIVATE | //single process use
                                       DB_INIT_LOCK | // Initialize locking
                                       DB_INIT_LOG | // Initialize logging
                                       DB_INIT_MPOOL | // Initialize the cache
                                       DB_INIT_TXN | // Initialize transactions
                                       DB_INIT_REP;
    ( ( DB_ENV* ) myEnv )->app_private = this;
    myEnv.set_alloc ( malloc, realloc, free );
    myEnv.set_cachesize ( 0, 128 * 1024 * 1024, 0 );
    myEnv.set_lg_bsize ( 1024 * 1024 * 64 );
    myEnv.set_timeout ( 15 * 1000, DB_SET_LOCK_TIMEOUT );
    myEnv.set_timeout ( 20 * 1000, DB_SET_TXN_TIMEOUT );
    myEnv.set_tx_max ( 64 * 200 );
    myEnv.set_lk_max_lockers ( 128 );
    myEnv.set_lk_max_locks ( 10000 );
    myEnv.set_lk_max_objects ( 10000 );
    myEnv.set_errcall ( db_errcall_fcn );
    myEnv.set_lg_max ( 30 );
    myEnv.set_thread_count ( 10 );
    myEnv.set_flags ( DB_TXN_NOSYNC, 1 );
    myEnv.set_event_notify ( CBackDb::db_event_callback );

    boost::regex addr_reg ( "(?<host>[^:]*):(?<port>.*)" );
    char dataEnvPath[] = "/tmp/dbtest";
    mkdir ( dataEnvPath, 0777 );
    myEnv.set_data_dir ( dataEnvPath );
    bool is_group_creator = config["group_creater"] == "true";
    DB_SITE* dbsite = NULL;
    std::string addr_str = config["selfsite"];
    if ( !addr_str.empty() ) {
        boost::smatch match;
        if ( boost::regex_match ( addr_str, match, addr_reg ) ) {
            int port = atoi ( match["port"].str().c_str() );
            std::string host = match["host"].str();
            myEnv.repmgr_site ( host.c_str(), port, &dbsite, 0 );

            dbsite->set_config ( dbsite, DB_LOCAL_SITE, 1 );
            if ( is_group_creator ) {
                dbsite->set_config ( dbsite, DB_GROUP_CREATOR, 1 );
            }
            dbsite->close ( dbsite );
            dbsite = NULL;
            printf ( "configed selfsite\n" );
        }
    }
    int wight = atoi ( config["wight"].c_str() );
    printf ( "wight:%d\n", wight );
    myEnv.rep_set_priority ( wight );
    myEnv.repmgr_set_ack_policy ( DB_REPMGR_ACKS_ONE_PEER );
    myEnv.rep_set_timeout ( DB_REP_ACK_TIMEOUT, 100 );
    myEnv.rep_set_timeout ( DB_REP_ELECTION_TIMEOUT, 5000 );
    myEnv.rep_set_timeout ( DB_REP_ELECTION_RETRY, 10 * 1000 );
    myEnv.rep_set_timeout ( DB_REP_CONNECTION_RETRY, 10 * 1000 );
    myEnv.rep_set_timeout ( DB_REP_HEARTBEAT_SEND, 15 * 1000 );
    myEnv.rep_set_timeout ( DB_REP_HEARTBEAT_MONITOR, 60 * 1000 );
    if ( !is_group_creator ) {
        addr_str = config["mastersite"];
        if ( !addr_str.empty() ) {
            boost::smatch match;
            if ( boost::regex_match ( addr_str, match, addr_reg ) ) {
                int port = atoi ( match["port"].str().c_str() );
                std::string host = match["host"].str();
                myEnv.repmgr_site ( host.c_str(), port, &dbsite, 0 );
                dbsite->set_config ( dbsite, DB_BOOTSTRAP_HELPER, 1 );
                dbsite->close ( dbsite );
                dbsite = NULL;
                printf ( "configed mastersite\n" );
            }
        }
    }

    try {
        myEnv.open ( dataEnvPath, env_flags, 0 );
        myEnv.repmgr_start ( 3, DB_REP_ELECTION );
        CreateDbConnect();
    } catch ( DbException e ) {
        printf ( "error:%s\n", e.what().c_str() );
    }
}
void CBackDb::CreateDbConnect()
{
    PDb t_maindb = new Db();
    PDb t_pointdb = new Db();
    DbTxn txn ( myEnv, NULL, 0 );
    try {
        t_maindb->create ( myEnv, 0 );
        t_pointdb->create ( myEnv, 0 );
        u_int32_t flags=DB_DUP;
        t_pointdb->set_flags ( flags );
        const u_int32_t dbopenflag = DB_CREATE | DB_MULTIVERSION;
        t_maindb->open ( txn, "maindb.db", "maindb", DB_BTREE, dbopenflag, 0 );
        t_pointdb->open ( txn, "maindb.db", "pointlist", DB_BTREE, dbopenflag, 0 );

        t_maindb->associate ( txn, t_pointdb->getHandle(), get_point_list,
                              DB_CREATE | DB_IMMUTABLE_KEY );
        txn.commit ( 0 );
        maindb = t_maindb;
        pointdb = t_pointdb;
    } catch ( DbException e ) {
        txn.abort();
        printf ( "create connect error:%s\n", e.what().c_str() );
    }
}
CBackDb::~CBackDb()
{
    pointdb = NULL;
    maindb = NULL;
    myEnv.close ( 0 );
}

void CBackDb::AddPoint ( int id, float lat, float lng )
{
    if ( canwrite == false ) {
        return;
    }
    PDb t_maindb = maindb;

    int idtmp = htonl ( id );
    unsigned char value_buffer[8];
    unsigned long long geocmb=GeoFun::CombineForGeo ( lat,lng );
    GeoFun::LongToByte ( geocmb,value_buffer );
    Dbt key ( &idtmp, sizeof ( idtmp ) );
    key.flags = DB_DBT_READONLY;
    Dbt value ( &value_buffer,sizeof ( value_buffer ) );
    value.flags=DB_DBT_READONLY;

    DbTxn txn ( myEnv, NULL, 0 );
    int dbres = t_maindb->put ( txn, &key, &value, 0 );
    txn.commit ( 0 );
    myEnv.txn_checkpoint ( 30 * 1024, 30, 0 );
}

void CBackDb::ReadAreaPoint ( float latmin, float latmax, float lngmin,
                              float lngmax, std::vector<GeoPoint>& list )
{
    assert ( lngmin<=lngmax );
    if ( lngmin * lngmax >= 0 ) {
        _ReadAreaPoint ( latmin, latmax, lngmin, lngmax, list );
    } else {
        if ( lngmax-lngmin<180 ) {
            _ReadAreaPoint ( latmin, latmax, lngmin, 0, list );
            _ReadAreaPoint ( latmin, latmax, 0, lngmax, list );
        } else {
            _ReadAreaPoint ( latmin, latmax,-180, lngmin, list );
            _ReadAreaPoint ( latmin, latmax, lngmax, 180, list );
        }
    }
}
void CBackDb::_ReadAreaPoint ( float latmin, float latmax, float lngmin,
                               float lngmax, std::vector<GeoPoint>& list )
{
    unsigned long long minpos = GeoFun::CombineForGeo ( latmin,lngmin );
    unsigned long long maxpos = GeoFun::CombineForGeo ( latmax,lngmax );

    PDb t_pointdb = pointdb;

    unsigned char indexbuf[8];
    unsigned char valuebuf[8];
    int id_buf=0;
    Dbt pkey ( indexbuf, sizeof ( indexbuf ) );
    pkey.flags = DB_DBT_USERMEM;
    Dbt srckey ( &id_buf,sizeof ( id_buf ) );
    srckey.flags = DB_DBT_USERMEM;
    Dbt srcvalue ( valuebuf,sizeof ( valuebuf ) );
    srcvalue.flags = DB_DBT_USERMEM;

    GeoFun::LongToByte ( minpos,indexbuf );
    Dbc dbc;
    dbc.cursor ( NULL,t_pointdb->getHandle(),0 );
    if ( dbc.pget ( &pkey,&srckey,&srcvalue,DB_SET_RANGE ) ==0 ) {
        do {
            unsigned long long pos=GeoFun::ByteToLong ( indexbuf );
            if ( pos>maxpos ) {
                break;
            }
            GeoPoint gp;
            gp.id = ntohl ( id_buf );
            GeoFun::SplitForGeo ( pos,gp.lat,gp.lng );
            if ( gp.lng>lngmin && gp.lng<lngmax && gp.lat>latmin && gp.lat<latmax ) {
                list.push_back ( gp );
            }
        } while ( dbc.pget ( &pkey,&srckey,&srcvalue,DB_NEXT ) ==0 );
    }
}

void CBackDb::db_event_callback ( DB_ENV* dbenv, u_int32_t which, void* info )
{
    ( ( CBackDb* ) dbenv->app_private )->OnDbEnent ( which, info );
}
void CBackDb::OnDbEnent ( u_int32_t which, void* info )
{
    info = NULL; /* Currently unused. */

    switch ( which ) {
    case DB_EVENT_REP_MASTER:
        printf ( "become master\n" );
        canwrite = true;
        break;

    case DB_EVENT_REP_CLIENT:
        printf ( "become client\n" );
        canwrite = false;
        break;
    case DB_EVENT_REP_PERM_FAILED:
        //pass
        break;
    case DB_EVENT_REP_STARTUPDONE: /* fallthrough */
    case DB_EVENT_REP_NEWMASTER:
        /* Ignore. */
        break;

    default:
        printf ( "ignoring event %d\n", which );
    }
}
void CBackDb::DoCheckPoint()
{
    //int res = 0;
    try {
        myEnv.txn_checkpoint ( 30 * 1024, 30, 0 );
        /*char **listp = nullptr;
         res = myEnv.log_archive(&listp, DB_ARCH_REMOVE);
         if (listp)
         free(listp);*/
    } catch ( DbException& e ) {
        printf ( "check point error:%s\n", e.what().c_str() );
    }
}
