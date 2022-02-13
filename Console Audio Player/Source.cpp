#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_thread.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include "Header.h"
#include <thread>
#include <algorithm>
#include <random>
#include <fstream>
#include <processthreadsapi.h>
#include <codecvt>


#define timeout8D 10



using namespace std;

bool LeaveThreads = false;

bool is8DActive = false;

string Location = ".\\";

void Player();
void _loop(float speed);
thread t1(Player);
thread loop;

SoLoud::Soloud soloud;
SoLoud::Wav _sample;


int CurrentAudio;

vector<filesystem::path> SongQueue;


#pragma region Commands
int ping(string a) {
	cout << "pong\n";
	return 0;
}

int move(string a) {
	if (a.empty()) {
		return -1;
	}
	if (filesystem::exists(a));
	if (a[1] == ':') {
		if (!filesystem::exists(a)){
			cout << "invalid path" << endl;
			return -1;
		}
		Location = a;
	} else {
		if (a[0] == '\\') {

			if (!filesystem::exists(Location + (a.substr(1, a.length() - 1)))) {
				cout << "invalid path" << endl;
				return -1;
			}

			Location.append(a.substr(1, a.length()-1));
		} else {

			if (!filesystem::exists(Location + a)) {
				cout << "invalid path" << endl;
				return -1;
			}

			Location.append(a);
		}
	}

	if (Location[Location.length() - 1] != '\\') {
		Location.push_back('\\');
	}

	return 0;
}

int leave(string a) {
	LeaveThreads = true;
	if (t1.joinable()) t1.join();
	if (loop.joinable()) loop.join();
	exit(0);
}

int dir(string a) {
	if (a.empty()) {
		system((char*)(("dir \"" + filesystem::absolute(Location).generic_string() + "\" /b").c_str()));
		cout << "dir \"" + filesystem::absolute(Location).generic_string() + "\" /b" << endl;;
	}
	else {
		system((char*)(("dir \"" + filesystem::absolute(Location).generic_string() + a + "\" /b").c_str()));
	}
	return 0;
}

int tree(string a) {
	system("tree");
	return 0;
}

int play(string a) {
	if (!a.empty()) {
		if (filesystem::exists(a));
		if (a[1] == ':') {
			if (!filesystem::exists(a)) {
				cout << "invalid path" << endl;
				return -1;
			}
			if (isFile(a)) {
				if (isAudio(a)) {
					SongQueue.push_back(filesystem::absolute(a).string());
				} else {
					cout << "invalid file type" << endl;
				}
			} else {
				for (filesystem::path p : filesystem::directory_iterator(a)) {
					if (isAudio(p.generic_string())) {
						SongQueue.push_back(p.string());
					}
				}
			}
		} else {
			if (a[0] == '\\') {

				if (!filesystem::exists(Location + (a.substr(1, a.length() - 1)))) {
					cout << "invalid path" << endl;
					return -1;
				}

				if (isFile(a)) {
					if (isAudio(a)) {
						SongQueue.push_back(filesystem::absolute((Location + (a.substr(1, a.length() - 1)))).string());
					} else {
						cout << "invalid file type" << endl;
					}
				} else {
					for (filesystem::path p : filesystem::directory_iterator(Location + (Location + (a.substr(1, a.length() - 1))))) {
						if (isAudio(p.generic_string())) {
							SongQueue.push_back(p.string());
						}
					}
				}
			} else {

				if (!filesystem::exists(Location + a)) {
					cout << "invalid path" << endl;
					return -1;
				}

				if (isFile(a)) {
					if (isAudio(a)) {
						SongQueue.push_back(filesystem::absolute((Location + a)).string());
					} else {
						cout << "invalid file type" << endl;
					}
				} else {
					for (filesystem::path p : filesystem::directory_iterator(Location + (Location + a))) {
						if (isAudio(p.generic_string())) {
							SongQueue.push_back(p.string());
						}
					}
				}
			}
		}
	} else {
		try {
			for (filesystem::path p : filesystem::directory_iterator(Location)) {

				//filesystem::rename(p, stripUnicode(p));
				//cout << tostringnoUTF(p) << endl;
				if (isAudio(p)) {
					SongQueue.push_back(p);
				}
			}
		} catch (const std::exception& exc) {
			cout << exc.what() << endl;
		}
	}


	return 0;
}

int Queue(string a) {
	int i = 0;
	for (filesystem::path q : SongQueue) {
		cout << i << "::" << tostringnoUTF(q) << endl;
		i++;
	}

	return 0;
}

int skip(string a) {
	soloud.stopAll();
	return 0;
}

int Volume(string a) {
	soloud.setGlobalVolume(stof(a));
	
	return 0;
}

int stop(string a) {
	SongQueue.clear();
	soloud.stopAll();
	return 0;
}

int _Shuffle(string a) {
	auto rng = std::default_random_engine{};
	std::shuffle(std::begin(SongQueue), std::end(SongQueue), rng);
	return 0;
}

int _System(string a) {
	system(a.c_str());
	return 0;
}

int _cls(string a) {
	system("cls");
	return 0;
}

int SoundRotation(string a){
	soloud.setPan(CurrentAudio, math::clamp(stof(a), -1, 1));
	return 0;
}

int Playlist(string a) {
	try {
		if (a.empty()) {
			for (filesystem::path pth : filesystem::directory_iterator(".\\playlists\\")) {
				string temp = (GetFileName(pth.generic_string()));
				temp.erase(temp.length() - 3, 3);
				cout << temp << endl;
			}
		} else {
			string command;
			string path;
			int i;

			for (i = 0; a[i] != ' ' && i < a.length(); i++) {
				command.push_back(a[i]);
			}

			//path = new char[a.length() - i - 2];
			for (i++; a[i] != ' ' && i < a.length(); i++) {
				path.push_back(a[i]);

			}

			if (command == "save") {

				filesystem::create_directory("playlists");

				wofstream file("./playlists/" + path + ".pl", ios::out);
				file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
				//cout << "./playlists/" + path + ".pl";

				if (!file) {
					cout << "Cannot open file!" << endl;
					return -1;
				}
				for (int i = 0; i < SongQueue.size(); i++) {
					file << SongQueue[i] << endl;
				}



			} else if (command == "load") {
				wifstream file(".\\playlists\\" + path + ".pl", ios::in);
				//SongQueue = (vector<filesystem::path>*)malloc(file.tellg());
				std::wstring line;
				while (std::getline(file, line)) {
					std::wistringstream iss(line);
					wstring temp;
					if (!(iss >> temp)) { break; }


					SongQueue.push_back(temp);
				}
			}
		}
		return 0;
	} catch (const std::exception& exc) {
		cout << endl << exc.what() << endl;
	}
}

int Eff8D(string a) {
	

	if (!a.empty()) {
		is8DActive = true;
		loop = thread(_loop, stof(a));
	} else {
		is8DActive = false;
	}
	if (!is8DActive) {
		soloud.setPan(CurrentAudio, 0);
		
		if (loop.joinable()) loop.join();
	}

	return 0;
}

int pause(string a) {
	//if(soloud.getPause(CurrentAudio)) hum_silently();
	SuspendThread(t1.native_handle());

	soloud.setPause(CurrentAudio, true);
	return 0;
}

int unpause(string a) {
	ResumeThread(t1.native_handle());
	soloud.setPause(CurrentAudio, false);
	return 0;
}

#pragma endregion


string ManDefinitions[][2] = {
	{"man", "-> manual page, duh"},
	{"help", "-> do you really wonder that... or are you just trolling?"},
	{ "shuffle", "-> shuffles the queue" },
	{"playlist", "-> lists all saved playlists"},
	{"playlist", " save [playlist name] -> saves playlist to file so you can easily load it when you want to"},
	{"playlist", " load [playlist name] -> loads playlist from file"},
	{"8D", "-> disables 8D audio"},
	{"8D", " [speed] -> Loops audio around your head in an 8D fashion with [speed] speed"},
	{"pause","you are definitelly trolling, IT PAUSES THE SONG"}
};

string Commands[] = { "man", "help", "ping", "cd", "exit", "quit", "leave", "dir", "tree","play", "queue", "skip", "volume", "v", "stop", "shuffle", "system", "cls", "rotation", "pan", "playlist", "playlists", "8D", "pause", "unpause"};
int man(string a) {
	if (a.empty()) {

		cout << "list of commands:\n";
		for (string b : Commands) {
			bool addmark = false;
			for (auto c : ManDefinitions) {
				if (c[0] == b) {
					addmark = true;
					break;
				}
			}
			cout << "\t" << (addmark ? "*|" : " |") << b << endl;
		}
	} else {
		cout << endl;
		for (auto b : ManDefinitions) {
			if (b[0] == a) {
				cout << b[0] << "::" << b[1] << endl;
			}
		}
		cout << endl;
	}

	return 0;
}
int ((*CommandFunctions[])(string)) = { man, man, ping, move, leave, leave , leave, dir, tree, play, Queue, skip, Volume, Volume, stop, _Shuffle, _System, _cls, SoundRotation, SoundRotation, Playlist, Playlist, Eff8D, pause, unpause};

void Player() {


	soloud.init();
	while (true) {
		if (!SongQueue.empty()) {
			_sample.load(SongQueue[0].wstring().c_str());
			CurrentAudio = soloud.play(_sample);

			SongQueue.erase(SongQueue.begin());
		}
		if (LeaveThreads) return;
		while (soloud.getActiveVoiceCount() > 0) {
			if (LeaveThreads) return;
			SoLoud::Thread::sleep(100);
		}
	}

	soloud.deinit();
}

void _loop(float speed) {

	bool state = false;
	while (true) {
		if (LeaveThreads) return;
		if (!is8DActive) return;

		if (!state) {
			soloud.setPan(CurrentAudio, math::clamp((soloud.getPan(CurrentAudio) + speed), -1, 1));
		} else {
			soloud.setPan(CurrentAudio, math::clamp((soloud.getPan(CurrentAudio) - speed), -1, 1));

		}
		if (soloud.getPan(CurrentAudio) == -1) {
			state = false;
		} else if (soloud.getPan(CurrentAudio) == 1) {
			state = true;
		}
		SoLoud::Thread::sleep(timeout8D);
	}
}

int main(int argc, char* argv[]) {

	
	int spacePos;

	string Command;

	

	cout << filesystem::absolute(Location) << ">>";
	getline(cin, Command);
	

	for (int i = 0; i < Command.size() && Command[i] != ' '; i++)spacePos = i+1;
	
	for (int i = 0; i < sizeof(Commands)/sizeof(Commands[0]); i++) {
		if (Command.substr(0, spacePos) == Commands[i]) {
			Command.erase(0, spacePos);
			if (!Command.empty()) {
				if (Command[0] == ' ') {
					Command.erase(0, 1);
				}
			}
			(*CommandFunctions[i])(Command);
		}
	}

	
	
	



	while (true) {
		cout << filesystem::absolute(Location) << ">>";
		getline(cin, Command);

		for (int i = 0; i < Command.size() && Command[i] != ' '; i++)spacePos = i + 1;

		for (int i = 0; i < sizeof(Commands) / sizeof(Commands[0]); i++) {
			if (Command.substr(0, spacePos) == Commands[i]) {
				Command.erase(0, spacePos);
				if (!Command.empty()) {
					if (Command[0] == ' ') {
						Command.erase(0, 1);
					}
				}
				(*CommandFunctions[i])(Command);
			}
		}
		
	}
	LeaveThreads = true;
	if (t1.joinable()) t1.join();
	if (loop.joinable()) loop.join();

	return 0;
}
