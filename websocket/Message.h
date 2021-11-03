#pragma once

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

using namespace std;

struct Message {
	int socketfd;
	string content;
};

#endif