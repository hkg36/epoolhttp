/*
 * BackDb.h
 *
 *  Created on: Apr 7, 2012
 *      Author: amen
 */
#include <string.h>
#include <stdlib.h>
#include <db.h>
#include "db_cpp.h"
#include <vector>
#ifndef BACKDB_H_
#define BACKDB_H_

struct GeoPoint
{
  int id;
  float lat,lng;
};
class CBackDb
{
private:
	DbEnv myEnv;
	PDb maindb;
	PDb pointdb;
	bool canwrite;
	static void db_errcall_fcn(const DB_ENV* dbenv,const char *errpfx,
			const char *msg);
	CBackDb();
	static void db_event_callback(DB_ENV *dbenv, u_int32_t which, void *info);
	void OnDbEnent(u_int32_t which, void *info);
	void CreateDbConnect();

	void _ReadAreaPoint(float latmin,float latmax,float lngmin,float lngmax,std::vector<GeoPoint> &list);
public:
	static CBackDb& Instanse();
	~CBackDb();
	void DoCheckPoint();

	void AddPoint(int id,float lat,float lng);
	void ReadAreaPoint(float latmin,float latmax,float lngmin,float lngmax,std::vector<GeoPoint> &list);
};
#endif /* BACKDB_H_ */
