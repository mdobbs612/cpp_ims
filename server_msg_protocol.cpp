#include "server.h"

const char * registerUser(const char *username, Client *client, int n_users);
const char * loginUser(const char *username, Client *client);
const char *logoutUser(Client *client);

Action_Msg::Action_Msg(ACTION_TYPE typ, const char * name, const char * target, const char * msg) : Msg(ACTION, name)
{
	action_type = typ;
	target_name = target;
	message = msg;
}

Confirm_Msg::Confirm_Msg(ACTION_TYPE typ, const char * name, const char * target) : Msg(CONFIRM, name)
{
	action_type = typ;
	target_name = target;
}

const char *Confirm_Msg::serialize() {
	char *buffer = new char[44];
	buffer[0] = CONFIRM;
	buffer[1] = action_type;

	const char *usr = Msg::get_username();
	int  len = strlen(usr);
	buffer[2] = (char)len + 1;
	for (int i = 0; i < len; i++) {
		buffer[i + 3] = usr[i];
	}

	const char *target = get_target();
	len = strlen(target);
	buffer[23] = (char)len + 1;
	for (int i = 0; i < len; i++) {
		buffer[24 + i] = target[i];
	}

	buffer[44] = '\0';
	return buffer;
}

Status_Msg::Status_Msg(const char * target, const char *name, ACTIVE_STATUS act, FRIEND_STATUS frnd) : Msg(STATUS, name)
{
	target_name = target;
	active_status = act;
	friend_status = frnd;
}

Err_Msg::Err_Msg(ERROR_TYPE err, const char * name) : Msg(ERR_MSG, name)
{
	error_type = err;
}

const char *Err_Msg::serialize() {
	char *buffer = new char[3];
	buffer[0] = ERR_MSG;
	buffer[1] = error_type;
	buffer[2] = '\0';
	return buffer;
}

Msg::Msg(MSG_TYPE t, const char *name)
{
	msg_type = t;
	username = name;
}



const char *ParseClientString(const char *buffer, Client *client) {   // Since the server only recieves action messages from user, probably maybe
	char t = buffer[0];
	int len = (int)buffer[1] - 1;

	char *username = new char[len + 1];
	for (int i = 0; i < len; i++) {
		username[i] = buffer[23 + i];
	}
	username[len] = '\0';

	len = (int)buffer[22] - 1;
	char *target_name = new char[len + 1];
	for (int i = 0; i < len; i++) {
		target_name[i] = buffer[23 + i];
	}
	target_name[len] = '\0';

	//const char *target_name = new const char[20];
	//const char *message = new const char[200];

	cout << "targ " << target_name << "other " << username << endl;

	int n_users = server_database.get_num_users();

	switch (t) {
		case('0') :  //REGISTER
			return registerUser(target_name, client, n_users);
			break;
		case('1') : //LOGIN																	NEEDS ACTION
			return loginUser(target_name, client);
			break;
		case('2') : //LOGOUT																	NEEDS ACTION
			logoutUser(client);
			break;

	} 

	return "";
}


const char * registerUser(const char *username, Client *client, int n_users) {
	cout << "Trying to register\n";
	if (client->is_logged_in()) {
		Err_Msg *msg;
		msg = new Err_Msg(ALREADY_LOGGED_IN, "");
		return msg->serialize();
	}
	else if (server_database.index_by_name(username) == -1) {
		server_database.insert_user(username, n_users);
		Confirm_Msg *msg;
		msg = new Confirm_Msg(REGISTER, username, "");
		return msg->serialize();
	}
	else {
		Err_Msg *msg;
		msg = new Err_Msg(USER_ALREADY_EXISTS, username);
		return msg->serialize();
	}
}

const char * loginUser(const char *username, Client *client) {
	cout << "Trying to login\n";
	if (client->is_logged_in()) {
		Err_Msg *msg;
		msg = new Err_Msg(ALREADY_LOGGED_IN, "");
		return msg->serialize();
	}
	else if (server_database.index_by_name(username) != -1) {
		Confirm_Msg *msg;
		msg = new Confirm_Msg(LOGIN, username, "");
		User *user = server_database.get_user_by_name(username);
		cout << "connecting: " << username << endl;
		client->connect_user(user);
		return msg->serialize();
	}
	else {
		Err_Msg *msg;
		msg = new Err_Msg(USER_DNE, username);
		return msg->serialize();
	}
}

const char *logoutUser(Client *client) {
	cout << "logging out " << endl;
	const char *curr_username = client->get_username();
	if (strlen(curr_username) >= 1) {
		client->logout_user();
		Confirm_Msg *msg;
		msg = new Confirm_Msg(LOGOUT, curr_username, "");
		return msg->serialize();
	}
	else {
		Err_Msg *msg;
		msg = new Err_Msg(NOT_LOGGED_IN, curr_username);
		return msg->serialize();
	}
}
