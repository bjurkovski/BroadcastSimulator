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

	public:
		int sender;
		int time;
		char content;

		Message() {
			id = -1;
			content = 0;
		}

		Message(int id, char content) {
			this->id = id;
			this->content = content;
			this->time = 0;
			this->sender = -1;
		}

		int getId() {
			return id;
		}

		int getTime() {
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
};

struct CompMessages :
public std::binary_function<Message, Message, bool> {
	bool operator() (const Message m1, const Message m2) const {
		if((m1.content == 'A') && (m2.content != 'A'))
			return true;
		else if((m1.content != 'A') && (m2.content == 'A'))
			return false;
		else if(m1.time == m2.time)
			return m1.sender < m2.sender;
		else
			return m1.time < m2.time;
	}
};

typedef std::priority_queue<Message, std::vector<Message>, CompMessages> MessageQueue;

class BroadcastSimulator {
	public:
		BroadcastSimulator() { }
		void initialize(std::string configFile);
		virtual void run() = 0;
	protected:
		int numProcs;
		int numMessages;
		std::vector< std::queue<int> > messagesPool;
		Message checkMessagePool(int proc, int round);
		void startSending(int proc, Message m);

		int currentBuffer;
		std::vector<Message> isSending;
		std::vector<int> procClock;
		std::vector<MessageQueue> procBuffer[2];
		void swapBuffers();

		std::map< int, std::queue<int> > msgDestinations;
		virtual bool send(int sender, int receiver, Message message);
		virtual Message receive(int receiver);
};

#endif
