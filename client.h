#pragma once 
#define UNICODE

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h> /*sleep*/
#include <pthread.h> 
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define BUFFER_LEN 250
#define DEFAULT_PORT "27012"
#define MAX_USERS 20
#define MAX_CLIENTS 20

using namespace std;

enum ACTIVE_STATUS { INACTIVE, ACTIVE };

// NOT = user2 is not user1's friend
// REQUESTED = user1 has already requested user2
// PENDING = user1 must answer user2's reqest
// YES = user1 and user2 are friends 
enum FRIEND_STATUS { NOT, REQUESTED, PENDING, YES };

class User {
	const char *username;
	int index;
	ACTIVE_STATUS active_status;

public:
	User(const char *name, int ind) : username(name), index(ind) { active_status = INACTIVE; }
	int get_index() { return index; }
	const char *get_name() { return username; }
};


class Database {
	int num_users;
	User *users[MAX_USERS];
	FRIEND_STATUS friend_matrix[MAX_USERS][MAX_USERS];

public:
	Database() { num_users = 0; }
	void insert_user(const char *name, int ind);
	const char *name_by_index(int ind);
	int index_by_name(const char *name);

	void request_friend(int user_ind, const char *friend_name);
	void accept_friend(int user_ind, const char *friend_name);
	//void reject_friend(const char *user, const char *friend); Can they keep trying? Notify friend? I dunno.
	// Should be alright to leave as NO and REQUESTED and they can accept when they'd like?

	FRIEND_STATUS friend_status(int user_ind, const char *friend_name);
	//const char **friend_list(const char *user); MAYBE make this just do the send message / ack and no return value

	User *get_user_by_name(const char *name);
	User *get_user_by_index(int ind);

};


class Client {
	//HANDLE client_thread; NOT SURE THAT THIS IS NECESSARY
	User *user_bound; // The user that is connected to server through this client (only after login success)
	pthread_t id; // thread id
	int client_socket;

public:
	//HANDLE get_handle() { return client_thread; }
	pthread_t get_id() { return id; }
	int get_client_socket() { return client_socket; }

	void connect_user(User *usr) { user_bound = usr; }

	Client() {
		id = -1;
		client_socket = -1;
	}

	Client(pthread_t thread_id, int s) { id = thread_id; client_socket = s; }

};

class Clients {
	int num_clients;
	Client **clients;

public:
	Clients() { num_clients = 0; clients = new Client*[MAX_CLIENTS]; };
	int new_client(Client *client) { clients[num_clients] = client; num_clients++; return 0; }
};




// ACT: An action is taken
// CONFIRM: Confirming that an act has been successful
// STATUS: Telling the user something relevant happened (Friend req, friend online, etc)
// ERROR: Something went wrong
enum MSG_TYPE { ACTION, CONFIRM, STATUS, ERROR_MSG };

// REGISTER: Register user with the server --- does not list them as online yet
// LOGIN/LOGOUT: Changes user's online status
// REQ/REM/ACPT/DENY_FRIEND: Alter's user's relationship to other player
// LIST_FRIENDS: Prints lit of user's friends for the user, including statuses and pendings
// MSG_FRIEND: Sends an IM to a specified friend of the user
enum ACTION_TYPE : char {
	REGISTER = '0',
	LOGIN = '1',
	LOGOUT = '2',
	ADD_FRIEND = '3',
	REM_FRIEND = '4',
	STATUS_ALL = '5',
	STATUS_ONE = '6',
	MSG_FRIEND = '7'
};

enum ERROR_TYPE {
	USER_ALREADY_EXISTS = '0',
	USER_DNE = '1',
	INVALID_NAME = '2',
	NOT_LOGGED_IN = '3',
	ALREADY_LOGGED_IN = '4'
};

// This is what will be sent to and from the clients.
class Msg {
	MSG_TYPE msg_type;
	const char *username; //required for some 9

public:
	//char *msg_to_str();
	//Msg (char *str);
	Msg(MSG_TYPE t, const char *name);
	MSG_TYPE get_msg_type() { return msg_type; }
	const char *get_username() { return username; }
};

class Action_Msg : public Msg {
	ACTION_TYPE action_type;
	const char *target_name;
	const char *message;				// for if the action is a message to send, empty otherwise

public:
	Action_Msg(ACTION_TYPE typ, const char *name, const char *target_name, const char *msg);
	ACTION_TYPE get_type() { return action_type; }
	const char *get_target() { return target_name; }

	const char *serialize();
};

class Confirm_Msg : public Msg {
	ACTION_TYPE action_type;	// Tells the message which action it is confirming has happened
	const char *username; // Does this need username????

public:
	Confirm_Msg(ACTION_TYPE typ, const char *name);
	ACTION_TYPE get_type() { return action_type; }

	const char *serialize(char *buffer) {
		buffer[0] = action_type;
		int  len = strlen(username);
		for (int i = 0; i < len; i++) {
			buffer[i + 1] = username[i];
		}
		cout << buffer;
		
		return buffer;
	};
};

class Status_Msg : public Msg {
	const char *username;						// In this case, this is the username of the person whose status is being reported
	ACTIVE_STATUS active_status;
	FRIEND_STATUS friend_status;

public:
	Status_Msg(const char *name, ACTIVE_STATUS act, FRIEND_STATUS frnd);
	const char *get_name() { return username; }
	ACTIVE_STATUS get_active_status() { return active_status; }
	FRIEND_STATUS get_friend_status() { return friend_status; }

	const char *serialize(char *buffer) {
		int  len = strlen(username);
		for (int i = 0; i < len; i++) {
			buffer[i] = username[i];
		}
		buffer[21] = active_status;
		buffer[22] = friend_status;
		return buffer;
	};
};

class Err_Msg : public Msg {
	ERROR_TYPE error_type;
	const char *username;

public:
	Err_Msg(ERROR_TYPE err, const char *name);
	const char *serialize(char *buffer) {
		buffer[0] = error_type;
		int  len = strlen(username);
		for (int i = 0; i < len; i++) {
			buffer[i + 1] = username[i];
		}
		return buffer;
	};
};


const char *SerializeMsg(Msg msg, const char *buffer);
void ParseString(const char *buffer);
void ParseConfirm(const char *buffer);
//void ParseStatus(const char *buffer);
void ParseError(const char *buffer);

const char *ParseInput(const char *cmd, const char* arg);
