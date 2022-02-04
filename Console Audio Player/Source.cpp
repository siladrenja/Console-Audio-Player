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



using namespace std;

bool LeaveThreads = false;

string Location = ".\\";

void Player();
thread t1(Player);

SoLoud::Soloud soloud;
SoLoud::Wav _sample;


int CurrentAudio;

vector<filesystem::path> SongQueue;


#pragma region Commands
int ping(string a) {
	cout << "pong";
	return 0;
}

int move(string a) {
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
	t1.join();
	exit(0);
}

int dir(string a) {

	system((char*)(("dir " + a + " /b").c_str()));

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
	cout << math::clamp(stof(a), -1, 1);
	return 0;
}

#pragma endregion


string ManDefinitions[][2] = {
	{"man", "manual page, duh"},
	{ "shuffle", "shuffles the queue" }

};

string Commands[] = { "man", "ping", "cd", "exit", "quit", "leave", "dir", "tree","play", "queue", "skip", "volume", "v", "stop", "shuffle", "system", "cls", "rotation", "pan"};
int man(string a) {
	if (a.empty()) {

		cout << "list of commands:\n";
		for (string b : Commands) {
			cout << "\t|" << b << endl;
		}
	} else {
		for (auto b : ManDefinitions) {
			if (b[0] == a) {
				cout << b[0] << "::" << b[1] << endl;
			}
		}
	}

	return 0;
}
int ((*CommandFunctions[])(string)) = { man, ping, move, leave, leave , leave, dir, tree, play, Queue, skip, Volume, Volume, stop, _Shuffle, _System, _cls, SoundRotation, SoundRotation };

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
	t1.join();

	return 0;
}
