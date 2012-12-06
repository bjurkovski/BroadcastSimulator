#ifndef BASIC_SIMULATOR_H
#define BASIC_SIMULATOR_H

#include <queue>
#include <vector>
#include <string>
#include <map>

class Message {
	private:
		int id;

	public:
		char content;

		Message() {
			id = -1;
			content = 0;
		}

		Message(int id, char content) {
			this->id = id;
			this->content = content;
		}

		int getId() {
			return id;
		}

		void clear() {
			id = -1;
			content = 0;
		}

		bool isNull() {
			if(id == -1)
				return true;
			return false;
		}
};

class BasicSimulator {
	public:
		BasicSimulator(std::string configFile);
		void run();
	protected:
		int numProcs;
		std::vector< std::queue<int> > messagesPool;
		void checkMessagePool(int round);

		int currentBuffer;
		std::vector<Message> isSending;
		std::vector<Message> procBuffer[2];
		void swapBuffers();

		std::map< int, std::queue<int> > msgDestinations;
		bool send(int sender, int receiver, Message message);
		Message receive(int receiver);
};

#endif
