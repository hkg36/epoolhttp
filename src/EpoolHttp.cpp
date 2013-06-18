#include "tools/CBuffer.h"
#include "db/BackDb.h"
#include <signal.h>
#include "Http/HttpServer.h"
#include "GeoHttpDb/HttpGeoDb.h"
CHttpHost server(8081);
void sigroutine(int unused) {
	server.Stop();
	printf("recv stop cmd\n");
}

int main() {
	signal(SIGINT, sigroutine);
	CBackDb::Instanse();
	printf("database inited\n");

	server.SetCreateFileTaskCallBack(GeoHttpTask::NewGeoHttpTask);
	server.Run();
	return 0;
}

