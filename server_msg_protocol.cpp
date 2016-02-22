#include "server.h"




Client * Clients::get_client_by_name(const char *name) {
  for(int i = 0; i < num_clients; i++) {
    if( strcmp(clients[i]->get_username(), name) == 0) return clients[i];
  }
  return nullptr;
}


int Clients::get_socket_by_name(const char *name) {
  for(int i = 0; i < num_clients; i++) {
    if( strcmp(clients[i]->get_username(), name) == 0) return clients[i]->get_client_socket();
  }
  return -1;
}


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
	for (int i = 0; i < 20; i++) {
    if (i < len) buffer[i + 3] = usr[i];
    else buffer [i + 3] = '#'; 
	}

	const char *target = get_target();
	len = strlen(target);
	buffer[23] = (char)len + 1;
	for (int i = 0; i < 20; i++) {
    if (i < len) buffer[i + 24] = target[i];
    else buffer [i + 24] = '#';
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



void sendToUser(const char *username, const char *msg) {
  if (userOnline(username) == ACTIVE) {
    Client *client = server_clients->get_client_by_name(username);
    int ConnectSocket = client->get_client_socket();
    int res = send(ConnectSocket, msg, strlen(msg), 0);
  }
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

	//cout << "> targ " << target_name << " other " << username << endl;

	int n_users = server_database.get_num_users();

	switch (t) {
		case('0') :  //REGISTER
			return registerUser(target_name, client, n_users);
			break;
		case('1') : //LOGIN																	NEEDS ACTION
			return loginUser(target_name, client);
			break;
		case('2') : //LOGOUT																	NEEDS ACTION
			return logoutUser(client);
			break;
    case('3') : //ADD FRIEND
      return addFriend(target_name, client);
      break;
    case('4') : //REMOVE FRIEND
      return remFriend(target_name, client);
      break;
    case('7') : //SEND MESSAGE
      return sendIM(target_name, client);
      break;
	} 

	return "";
}


ACTIVE_STATUS userOnline(const char *username) {
  return server_database.get_user_by_name(username)->get_status();
}


const char * registerUser(const char *username, Client *client, int n_users) {
  
	if (client->is_logged_in()) {
		Err_Msg *msg;
		msg = new Err_Msg(ALREADY_LOGGED_IN, "");
		return msg->serialize();
	}
	else if (server_database.index_by_name(username) == -1) {
    //cout << "> It was -1" << endl;
		server_database.insert_user(username, n_users);
		Confirm_Msg *msg;
		msg = new Confirm_Msg(REGISTER, username, "");
    cout << "> Sending: " << msg->serialize() << endl;
		return msg->serialize();
	}
	else {
		Err_Msg *msg;
		msg = new Err_Msg(USER_ALREADY_EXISTS, username);
		return msg->serialize();
	}
}

const char * loginUser(const char *username, Client *client) {
  cout << "ACTIVE STATUS IS " << userOnline(username) << endl;
	if (client->is_logged_in()) {
		Err_Msg *msg;
		msg = new Err_Msg(ALREADY_LOGGED_IN, "");
		return msg->serialize();
	}
  else if (userOnline(username) == ACTIVE) {
    Err_Msg *msg;
		msg = new Err_Msg(ALREADY_LOGGED_IN, "");
		return msg->serialize();
  }
	else if (server_database.index_by_name(username) != -1) {
		Confirm_Msg *msg;
		msg = new Confirm_Msg(LOGIN, username, "");
		User *user = server_database.get_user_by_name(username);
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
  cout << "Called logoutUser\n";
	//const char *curr_username = client->get_username();
  cout << "Is logged in: " << client->is_logged_in() << endl;
	if (client->is_logged_in() == 1) {
		client->logout_user();
		Confirm_Msg *msg;
		msg = new Confirm_Msg(LOGOUT, "", "");
		return msg->serialize();
	}
	else {
		Err_Msg *msg;
		msg = new Err_Msg(NOT_LOGGED_IN, "");
		return msg->serialize();
	}
}

const char * addFriend(const char *target_name, Client *client) {
  const char *username = client->get_username();
  int user_index = server_database.index_by_name(username);
  int fr_index = server_database.index_by_name(target_name);
  Err_Msg *emsg;
  Confirm_Msg *msg;

  if (fr_index == -1) {
		emsg = new Err_Msg(USER_DNE, target_name);
		return emsg->serialize();
  } 
  else if (strcmp (username, target_name) == 0) {
    emsg = new Err_Msg(ADD_SELF, target_name);
    return emsg->serialize();
  }
  else {
    FRIEND_STATUS fr_status = server_database.friend_status(user_index, target_name);

    switch(fr_status) {
      case (NOT) : 
        server_database.request_friend(user_index, target_name);
        msg = new Confirm_Msg(ADD_FRIEND, username, target_name);
		    return msg->serialize();
        //send confirmation messages
        break;
      case (REQUESTED) :
		    emsg = new Err_Msg(ALREADY_REQUESTED, target_name);
		    return emsg->serialize();
        break;
      case (PENDING) : 
        server_database.accept_friend(user_index, target_name);
        msg = new Confirm_Msg(ACPT_FRIEND, username, target_name);
        return msg->serialize();
        break;
      case (YES) : 
		    emsg = new Err_Msg(ALREADY_FRIENDS, target_name);
		    return emsg->serialize();
        break;
	    }
    }
   return "hello\n";
}


const char * remFriend(const char *target_name, Client *client) {
  const char *username = client->get_username();
  int user_index = server_database.index_by_name(username);
  int fr_index = server_database.index_by_name(target_name);
  Err_Msg *emsg;
  Confirm_Msg *msg;

  if (fr_index == -1) {
		emsg = new Err_Msg(USER_DNE, target_name);
		return emsg->serialize();
  } 
  else if (strcmp (username, target_name) == 0) {
    emsg = new Err_Msg(ADD_SELF, target_name);
    return emsg->serialize();
  }
  else {
    FRIEND_STATUS fr_status = server_database.friend_status(user_index, target_name);

    switch(fr_status) {
      case (NOT) : 
        emsg = new Err_Msg(NOT_FRIENDS, target_name);
		    return emsg->serialize();
        break;
      case (REQUESTED) :
		    server_database.remove_friend(user_index, target_name);
        msg = new Confirm_Msg(ADD_FRIEND, username, target_name);
		    return msg->serialize();
        break;
      case (PENDING) : 
        server_database.remove_friend(user_index, target_name);
        msg = new Confirm_Msg(DENY_FRIEND, username, target_name);
        return msg->serialize();
        break;
      case (YES) : 
		    server_database.remove_friend(user_index, target_name);
        msg = new Confirm_Msg(REM_FRIEND, username, target_name);
        return msg->serialize();
        break;
	    }
    }
   return "hello\n";
}


const char * sendIM(const char * target_name, Client * client) {
  const char *username = client->get_username();
}

