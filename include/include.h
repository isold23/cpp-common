#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <mqueue.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <list>
#include <map>
#include <vector>
#include <cassert>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <memory>
#include <cassert>

using std::fstream;
using std::vector;
using std::map;
using std::list;

#include "define.h"
#include "standard_serialize.h"
#include "critical_section.h"
#include "debugtrace.h"
#include "common.h"

#endif //_INCLUDE_H_






