#ifndef BROADCAST_SIMULATOR_H
#define BROADCAST_SIMULATOR_H

#include <queue>
#include <vector>
#include <string>
#include <map>
#include <functional>

class Message {
	private:
		int id;
		int creator;
	public:
		int sender;
		int time;
		char content;

		Message() {
			id = -1;
			content = 0;
			creator = -1;
		}

		Message(int id, int creator, char content) {
			this->id = id;
			this->creator = creator;
			this->content = content;
			this->time = 0;
			this->sender = -1;
		}

		int getId() const {
			return id;
		}

		int getCreator() const {
			return creator;
		}

		int getTime() const {
			return time;
		}

		void clear() {
			id = -1;
			content = 0;
			time = 0;
		}

		bool isNull() {
			if(id == -1)
				return true;
			return false;
		}

		bool operator<(const Message& m) const {
			if(time == m.time)
				return creator < m.creator;
			else return time < m.time;
		}
};

struct CompMessages :
public std::binary_function<Message, Message, bool> {
	bool operator() (const Message m1, const Message m2) const {
		return m2 < m1;
	}
};

typedef std::priority_queue<Message, std::vector<Message>, CompMessages> MessageQueue;

class BroadcastSimulator {
	public:
		BroadcastSimulator() { }
		void initialize(std::string configFile);
		void run();
	protected:
		int numProcs;
		int numMessages;
		std::vector< std::queue<int> > messagesPool;
		Message checkMessagePool(int proc, int round);
		void startSending(int proc, Message m);

		int currentBuffer;
		std::vector< std::queue<Message> > isSending;
		std::vector<int> procClock;
		std::vector<MessageQueue> procBuffer[2];
		void swapBuffers();
		virtual bool hasMessageToReceive();

		std::map< int, std::queue<int> > msgDestinations;
		virtual bool send(int sender, int receiver, Message message);
		virtual Message receive(int receiver);
		virtual bool broadcast(int round) = 0;
};

#endif
