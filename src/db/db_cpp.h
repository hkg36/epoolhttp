#pragma once
#include <db.h>
#include <string>
#include <boost/concept_check.hpp>
#include "../tools/IPtr.h"

#ifdef DBNOTHROW
#define DBTHROW(error)
#else
#define DBTHROW(error) if(error!=0){throw DbException(error,__FUNCTION__);}
#endif
class DbException {
private:
	int errorcode;
	const char *codepos;
public:
	DbException(int errorcode, const char *codepos) :
		errorcode(errorcode), codepos(codepos) {
	}
	const std::string what() const {
		char buffer[128];
		sprintf(buffer, "At %s => %s", codepos, db_strerror(errorcode));
		return buffer;
	}
};
class DbTxn {
	friend class DbEnv;
private:
	DB_TXN *txn;
	DbTxn(DB_TXN *t) :
		txn(t) {
	}
	DbTxn(const DbTxn&) {
	}
	void operator=(const DbTxn&) {
	}
public:
	DbTxn(DB_ENV *env, DB_TXN *parent, u_int32_t flags) {
		int res = env->txn_begin(env, parent, &txn, flags);
		DBTHROW(res);
	}
	operator DB_TXN *() const {
		return txn;
	}
	inline ~DbTxn() {
		abort();
	}
	inline int abort() {
		if (txn) {
			int res = txn->abort(txn);
			DBTHROW(res);
			txn = NULL;
			return res;
		}
		return 0;
	}
	inline int commit(u_int32_t flags) {
		if (txn) {
			int res = txn->commit(txn, flags);
			DBTHROW(res);
			txn = NULL;
			return res;
		}
		return 0;
	}
};
class DbEnv {
private:
	DB_ENV * env;
	int res;
	DbEnv(const DbEnv&) {
	}
	void operator=(const DbEnv& a) {
	}
public:
	inline operator DB_ENV *() const {
		return env;
	}
	inline DbEnv(u_int32_t flag) :
		env(NULL) {
		res = db_env_create(&env, flag);
	}
	inline int close(u_int32_t flags) {
		if (env) {
			int res = env->close(env, flags);
			DBTHROW(res);
			env = NULL;
			return res;
		}
		return 0;
	}
	inline ~DbEnv() {
		close(0);
	}
	inline int set_alloc(void*(*_malloc)(size_t),
			void*(*_ralloc)(void*, size_t), void(*_free)(void*)) {
		int res = env->set_alloc(env, _malloc, _ralloc, _free);
		DBTHROW(res);
		return res;
	}
	inline int set_cachesize(u_int32_t gbytes, u_int32_t bytes, int ncache) {
		int res = env->set_cachesize(env, gbytes, bytes, ncache);
		DBTHROW(res);
		return res;
	}
	inline int open(const char *db_home, u_int32_t flags, int mode) {
		int res = env->open(env, db_home, flags, mode);
		DBTHROW(res);
		return res;
	}
	inline void set_errcall(
			void(*db_errcall_fcn)(const DB_ENV *dbenv, const char *errpfx,
					const char *msg)) {
		env->set_errcall(env, db_errcall_fcn);
	}
	inline int set_event_notify(
			void(*db_event_fcn)(DB_ENV *dbenv, u_int32_t event,
					void *event_info)) {
		return env->set_event_notify(env, db_event_fcn);
	}
	inline void set_errpfx(const char *errpfx) {
		env->set_errpfx(env, errpfx);
	}
	inline int set_tx_max(u_int32_t max) {
		return env->set_tx_max(env, max);
	}
	inline int set_timeout(db_timeout_t timeout, u_int32_t flags) {
		int res = env->set_timeout(env, timeout, flags);
		DBTHROW(res);
		return res;
	}
	inline int txn_checkpoint(u_int32_t kbyte, u_int32_t min, u_int32_t flags) {
		int res = env->txn_checkpoint(env, kbyte, min, flags);
		DBTHROW(res);
		return res;
	}
	inline int log_archive(char ***listp, u_int32_t flags) {
		int res = env->log_archive(env, listp, flags);
		DBTHROW(res);
		return res;
	}
	inline int set_lg_bsize(u_int32_t lg_bsize) {
		int res = env->set_lg_bsize(env, lg_bsize);
		DBTHROW(res);
		return res;
	}
	inline int lock_detect(u_int32_t atype, int *rejected) {
		int res = env->lock_detect(env, 0, atype, rejected);
		DBTHROW(res);
		return res;
	}
	inline int set_lk_max_lockers(u_int32_t max) {
		return env->set_lk_max_lockers(env, max);
	}
	inline int set_lk_max_locks(u_int32_t max) {
		return env->set_lk_max_locks(env, max);
	}
	inline int set_lk_max_objects(u_int32_t max) {
		return env->set_lk_max_objects(env, max);
	}
	inline DbTxn cdsgroup_begin() {
		DB_TXN *tid = NULL;
		int res = env->cdsgroup_begin(env, &tid);
		DBTHROW(res);
		return tid;
	}
	inline int set_lg_max(u_int32_t lg_max_M) {
		return env->set_lg_max(env, lg_max_M * 1024 * 1024);
	}
	inline int set_thread_count(u_int32_t count) {
		return env->set_thread_count(env, count);
	}
	u_int32_t get_thread_count() {
		u_int32_t count = 0;
		env->get_thread_count(env, &count);
		return count;
	}
	inline void set_isalive(
			int(*is_alive)(DB_ENV *dbenv, pid_t pid, db_threadid_t tid,
					u_int32_t flags)) {
		int res = env->set_isalive(env, is_alive);
		DBTHROW(res);
	}
	inline void set_thread_id(
			void(*thread_id)(DB_ENV *dbenv, pid_t *pid, db_threadid_t *tid)) {
		int res = env->set_thread_id(env, thread_id);
		DBTHROW(res);
	}
	void set_data_dir(const char *path)
	{
		int res=env->set_data_dir(env,path);
		DBTHROW(res);
	}
	void set_flags(u_int32_t name,int value)
	{
		int res=env->set_flags(env,name,value);
		DBTHROW(res)
	}
	void repmgr_site(const char * host, u_int port, DB_SITE** site, u_int32_t flag)
	{
		int res=env->repmgr_site(env,host,port,site,flag);
		DBTHROW(res)
	}
	void rep_set_priority(int priority)
	{
		int res=env->rep_set_priority(env,priority);
		DBTHROW(res)
	}
	void repmgr_start(int threadnum,u_int32_t flag)
	{
		int res=env->repmgr_start(env,threadnum,flag);
		DBTHROW(res)
	}
	void repmgr_set_ack_policy(int flag)
	{
		int res=env->repmgr_set_ack_policy(env,flag);
		DBTHROW(res)
	}
	void rep_set_timeout(int flag, db_timeout_t time)
	{
		int res=env->rep_set_timeout(env,flag,time);
		DBTHROW(res)
	}
};
struct Dbt: public DBT {
	Dbt(void *data, u_int32_t size) {
		memset(this, 0, sizeof(DBT));
		this->data = data;
		this->size = size;
		this->ulen = size;
	}
	Dbt() {
		memset(this, 0, sizeof(DBT));
	}
};
class Dbc {
	friend class Db;
private:
	DBC *dbc;
	Dbc(const DBC &) {
	}
	void operator=(const DBC &) {
	}
public:
	Dbc()
	{
	  dbc=NULL;
	}
	inline int cursor(DB_TXN *txnid,DB *db,u_int32_t flags) {
		int res = db->cursor(db, txnid, &dbc, flags);
		DBTHROW(res);
		return res;
	}
	inline ~Dbc() {
		close();
	}
	inline int close() {
		if (dbc) {
			int res = dbc->close(dbc);
			DBTHROW(res);
			dbc = NULL;
			return res;
		}
		return 0;
	}
	inline int get(DBT *key, DBT *data, u_int32_t flags) {
		int res = dbc->get(dbc, key, data, flags);
		return res;
	}
	inline int pget(DBT *key, DBT *pkey, DBT *data, u_int32_t flags) {
		int res = dbc->pget(dbc, key, pkey, data, flags);
		return res;
	}
	int put(DBC *DBcursor, DBT *key, DBT *data, u_int32_t flags) {
		int res = dbc->put(dbc, key, data, flags);
		DBTHROW(res);
		return res;
	}
	inline int del(u_int32_t flags) {
		int res = dbc->del(dbc, flags);
		DBTHROW(res);
		return res;
	}
	inline bool cmp(DBC *other_cursor) {
		int cmpres = 0;
		int res = dbc->cmp(dbc, other_cursor, &cmpres, 0);
		DBTHROW(res);
		return cmpres == 0;
	}
	inline db_recno_t count() {
		db_recno_t recno = 0;
		int res = dbc->count(dbc, &recno, 0);
		DBTHROW(res);
		return recno;
	}
	Dbc(DBC* other,u_int32_t flags) {
		int res = other->dup(other, &dbc, flags);
		DBTHROW(res);
	}
	inline int set_priority(DB_CACHE_PRIORITY priority) {
		int res = dbc->set_priority(dbc, priority);
		DBTHROW(res);
		return res;
	}
	inline DB_CACHE_PRIORITY get_priority() {
		DB_CACHE_PRIORITY cpres = DB_PRIORITY_UNCHANGED;
		int res = dbc->get_priority(dbc, &cpres);
		DBTHROW(res);
		return cpres;
	}
};
class Db:public IPtrBase<Db> {
private:
	DB *db;
	Db(const Db&) {
	}
	void operator=(const Db&) {
	}
public:
	inline DB* getHandle() const {
		return db;
	}
	inline Db() :
		db(NULL) {
	}
	inline int create(DB_ENV *dbenv, u_int32_t flags) {
		int res = db_create(&db, dbenv, flags);
		DBTHROW(res);
		return res;
	}
	inline int close(u_int32_t flags) {
		if (db) {
			int res = db->close(db, flags);
			DBTHROW(res);
			db = NULL;
			return res;
		}
		return 0;
	}
	inline Db(DB_ENV *dbenv, u_int32_t flags) :
		db(NULL) {
		create(dbenv, flags);
	}
	inline ~Db() {
		close(0);
	}
	inline int set_flags(u_int32_t flags) {
		int res = db->set_flags(db, flags);
		DBTHROW(res);
		return res;
	}
	inline int open(DB_TXN *txnid, const char *file, const char *database,
			DBTYPE type, u_int32_t flags, int mode) {
		int res = db->open(db, txnid, file, database, type, flags, mode);
		DBTHROW(res);
		return res;
	}
	inline int associate(
			DB_TXN *txnid,
			DB *secondary,
			int(*callback)(DB *secondary, const DBT *key, const DBT *data,
					DBT *result), u_int32_t flags) {
		int res = db->associate(db, txnid, secondary, callback, flags);
		DBTHROW(res);
		return res;
	}
	inline int put(DB_TXN *txnid, DBT *key, DBT *data, u_int32_t flags) {
		int res = db->put(db, txnid, key, data, flags);
		return res;
	}
	inline int get(DB_TXN *txnid, DBT *key, DBT *data, u_int32_t flags) {
		int res = db->get(db, txnid, key, data, flags);
		return res;
	}
	inline int pget(DB_TXN *txnid, DBT *key, DBT *pkey, DBT *value,
			u_int32_t flags) {
		int res = db->pget(db, txnid, key, pkey, value, flags);
		return res;
	}
	inline int del(DB_TXN *txnid, DBT *key, u_int32_t flags) {
		int res = db->del(db, txnid, key, flags);
		return res;
	}
	inline int sync(u_int32_t flags) {
		int res = db->sync(db, flags);
		DBTHROW(res);
		return res;
	}
	inline int associate_foreign(
			DB *secondary,
			int(*callback)(DB *secondary, const DBT *key, DBT *data,
					const DBT *foreignkey, int *changed), u_int32_t flags) {
		int res = db->associate_foreign(db, secondary, callback, flags);
		DBTHROW(res);
		return res;
	}
	inline int compact(DB_TXN *txnid, DBT *start = NULL, DBT *stop = NULL,
			DB_COMPACT *c_data = NULL, u_int32_t flags = 0, DBT *end = NULL) {
		int res = db->compact(db, txnid, start, stop, c_data, flags, end);
		DBTHROW(res);
		return res;
	}
	inline int exists(DB_TXN *txnid, DBT *key, u_int32_t flags) {
		return db->exists(db, txnid, key, flags);
	}
	inline int get_multiple() {
		return db->get_multiple(db);
	}
	inline int remove(const char *file, const char *database) {
		int res = db->remove(db, file, database, 0);
		DBTHROW(res);
		return res;
	}
	inline int rename(const char *file, const char *database,
			const char *newname) {
		int res = db->rename(db, file, database, newname, 0);
		DBTHROW(res);
		return res;
	}
	inline int set_priority(DB_CACHE_PRIORITY priority) {
		int res = db->set_priority(db, priority);
		DBTHROW(res);
		return res;
	}
	inline DB_CACHE_PRIORITY get_priority() {
		DB_CACHE_PRIORITY priority = DB_PRIORITY_UNCHANGED;
		int res = db->get_priority(db, &priority);
		DBTHROW(res);
		return priority;
	}
	inline u_int32_t get_flags(){
	  u_int32_t flags;
	  int res=db->get_flags(db,&flags);
	  DBTHROW(res);
	  return flags;
	}
};
typedef CIPtr<Db> PDb;
class DbMultipleIterator {
public:
	DbMultipleIterator(const Dbt &dbt) :
		data_((u_int8_t*) dbt.data),
				p_((u_int32_t*) (data_ + dbt.ulen - sizeof(u_int32_t))) {
	}
protected:
	u_int8_t *data_;
	u_int32_t *p_;
};

class DbMultipleKeyDataIterator: private DbMultipleIterator {
public:
	DbMultipleKeyDataIterator(const Dbt &dbt) :
		DbMultipleIterator(dbt) {
	}
	bool next(Dbt &key, Dbt &data) {
		if (*p_ == (u_int32_t) -1) {
			key.data = (0);
			key.size = (0);
			data.data = (0);
			data.size = (0);
			p_ = 0;
		} else {
			key.data = (data_ + *p_--);
			key.size = (*p_--);
			data.data = (data_ + *p_--);
			data.size = (*p_--);
		}
		return (p_ != 0);
	}
};

class DbMultipleRecnoDataIterator: private DbMultipleIterator {
public:
	DbMultipleRecnoDataIterator(const Dbt &dbt) :
		DbMultipleIterator(dbt) {
	}
	bool next(db_recno_t &recno, Dbt &data) {
		if (*p_ == (u_int32_t) 0) {
			recno = 0;
			data.data = (0);
			data.size = (0);
			p_ = 0;
		} else {
			recno = *p_--;
			data.data = (data_ + *p_--);
			data.size = (*p_--);
		}
		return (p_ != 0);
	}
};

class DbMultipleDataIterator: private DbMultipleIterator {
public:
	DbMultipleDataIterator(const Dbt &dbt) :
		DbMultipleIterator(dbt) {
	}
	bool next(Dbt &data) {
		if (*p_ == (u_int32_t) -1) {
			data.data = (0);
			data.size = (0);
			p_ = 0;
		} else {
			data.data = (data_ + *p_--);
			data.size = (*p_--);
			if (data.size == 0 && data.data == data_)
				data.data = (0);
		}
		return (p_ != 0);
	}
};

class DbMultipleBuilder {
public:
	DbMultipleBuilder(Dbt &dbt) :
		dbt_(dbt) {
		DB_MULTIPLE_WRITE_INIT(p_, &dbt_);
	}
protected:
	Dbt &dbt_;
	void *p_;
};

class DbMultipleDataBuilder: DbMultipleBuilder {
public:
	DbMultipleDataBuilder(Dbt &dbt) :
		DbMultipleBuilder(dbt) {
	}
	bool append(void *dbuf, size_t dlen) {
		DB_MULTIPLE_WRITE_NEXT(p_, &dbt_, dbuf, dlen);
		return (p_ != 0);
	}
	bool reserve(void *&ddest, size_t dlen) {
		DB_MULTIPLE_RESERVE_NEXT(p_, &dbt_, ddest, dlen);
		return (ddest != 0);
	}
};

class DbMultipleKeyDataBuilder: DbMultipleBuilder {
public:
	DbMultipleKeyDataBuilder(Dbt &dbt) :
		DbMultipleBuilder(dbt) {
	}
	bool append(void *kbuf, size_t klen, void *dbuf, size_t dlen) {
		DB_MULTIPLE_KEY_WRITE_NEXT(p_, &dbt_,
				kbuf, klen, dbuf, dlen);
		return (p_ != 0);
	}
	bool reserve(void *&kdest, size_t klen, void *&ddest, size_t dlen) {
		DB_MULTIPLE_KEY_RESERVE_NEXT(p_, &dbt_,
				kdest, klen, ddest, dlen);
		return (kdest != 0 && ddest != 0);
	}
};

class DbMultipleRecnoDataBuilder {
public:
	DbMultipleRecnoDataBuilder(Dbt &dbt) :
		dbt_(dbt) {
		DB_MULTIPLE_RECNO_WRITE_INIT(p_, &dbt_);
	}
	bool append(db_recno_t recno, void *dbuf, size_t dlen) {
		DB_MULTIPLE_RECNO_WRITE_NEXT(p_, &dbt_,
				recno, dbuf, dlen);
		return (p_ != 0);
	}

	bool reserve(db_recno_t recno, void *&ddest, size_t dlen) {
		DB_MULTIPLE_RECNO_RESERVE_NEXT(p_, &dbt_,
				recno, ddest, dlen);
		return (ddest != 0);
	}
protected:
	Dbt &dbt_;
	void *p_;
};
#undef DBTHROW
