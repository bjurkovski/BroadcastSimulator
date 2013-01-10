#ifndef SIM_MESSAGE_H
#define SIM_MESSAGE_H

#include <vector>
#include <queue>
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

#endif
