#include "SSP_Server.h"

void cSSP_Server::genInfoV() {
	for (const auto &xSer : info)
		infoV.push_back(make_pair(xSer.first, xSer.second));
}