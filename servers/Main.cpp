#include "Config.h"
#include "ServerTypes.h"
#include "FrameWork.h"
#include <stdio.h>

using namespace Server;
int main(int argc, char **argv) {
	Server::ConnectFrame frame;

	frame.init();
	frame.working();

	return 0;
}
