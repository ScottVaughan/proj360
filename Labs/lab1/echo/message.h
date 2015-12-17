#include <string>
class Message {
	public:
		std::string command;
		//std::string params[];
		std::string value;
		std::string cache;//i added
		bool needed;
};
