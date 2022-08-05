#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_thread.h"
#include "soloud_bassboostfilter.h"
#include "soloud_freeverbfilter.h"
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
//#include <processthreadsapi.h>
#include <codecvt>
#include <unordered_map>

#define timeout8D 10

bool threadPause = false;

using namespace std;

bool LeaveThreads = false;

bool is8DActive = false;

string Location = ".\\";

void Player();
void _loop(float speed);
thread t1(Player);
thread loop;
thread ytDlThrd;

bool ytDlThrdJoinable = false;

SoLoud::Soloud soloud;
SoLoud::Wav _sample;


int CurrentAudio;

vector<filesystem::path> SongQueue;
unsigned long long SongNUM = 0;

int unpause(string);
int play(string);

void ytDl(string a) {
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	stringstream strstr;
	strstr << std::put_time(&tm, "ytdl-%d-%m-%Y-%H-%M-%S");
	string dat = strstr.str();
	//cout << (string("ytdl.exe -x --audio-format \"mp3\" -o '" + dat + "/%(title)s-%(id)s.%(ext)s'") + "\"" + a + "\"");
	system( (string("ytdl.exe -i -x --audio-format \"mp3\" -o " + dat + "/%(title)s-%(id)s.%(ext)s") + " \""+ a + "\"").c_str());
	//cout << "got this far2";

	play("./" + dat);
	ytDlThrdJoinable = true;
	return;
}

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
					if (isAudio(p.generic_wstring())) {
						SongQueue.push_back(p);
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
						SongQueue.push_back(filesystem::absolute((Location + (a.substr(1, a.length() - 1)))));
					} else {
						cout << "invalid file type" << endl;
					}
				} else {
					for (filesystem::path p : filesystem::directory_iterator(Location + (Location + (a.substr(1, a.length() - 1))))) {
						if (isAudio(p.generic_string())) {
							SongQueue.push_back(p);
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
						SongQueue.push_back(filesystem::absolute((Location + a)));
					} else {
						cout << "invalid file type" << endl;
					}
				} else {
					for (filesystem::path p : filesystem::directory_iterator(Location + (Location + a))) {
						if (isAudio(p.wstring())) {
							SongQueue.push_back(p);
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

	unpause("");
	return 0;
}

int ytPlay(string a) {
	//if (ytDlThrd.joinable())ytDlThrd.join();
	//ytDlThrd = thread(ytDl, a);
	ytDl(a);
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

int seek(string a) {
	if (!a.empty()) {
		soloud.seek(CurrentAudio, stof(a));
		return 0;
	}
	return -1;
}

int previous(string a) {
	if (SongNUM < 1) {
		seek("0");
	}
	SongNUM-= 2;
	soloud.stopAll();
	return 0;
}

int SongGoTo(string a) {
	try {
		SongNUM = stoi(a);

		if (SongNUM >= SongQueue.size()) {
			SongNUM = SongQueue.size() - 1;
		}
	} catch (...) {

	}
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
	/*auto rd = std::random_device{};
	auto rng = std::default_random_engine{ rd() };
	std::shuffle(std::begin(SongQueue), std::end(SongQueue), rng);*/

	//random_shuffle(std::begin(SongQueue), std::end(SongQueue));

	std::random_device rd;
	std::mt19937 g(rd());

	std::shuffle(SongQueue.begin(), SongQueue.end(), g);

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
					file << SongQueue[i].wstring() << endl;
				}



			} else if (command == "load") {
				std::wifstream wif(".\\playlists\\" + path + ".pl");
				wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
				std::wstringstream wss;
				wss << wif.rdbuf();
				std::wstring line;
				while (getline(wss, line, L'\n')) {
					SongQueue.push_back(line);

				}
				unpause(string());
			}
		}
		return 0;
	} catch (const std::exception& exc) {
		cout << endl << exc.what() << endl;
	}
}

int Eff8D(string a) {
	

	if (!a.empty()) {
		is8DActive = false;
		if (loop.joinable()) loop.join();
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
	//SuspendThread(t1.native_handle());
	threadPause = true;
	soloud.setPause(CurrentAudio, true);
	return 0;
}

int unpause(string a) {
	
	soloud.setPause(CurrentAudio, false);
	//SoLoud::Thread::sleep(5);
	threadPause = false;
	return 0;
}

int Bass(string a) {
	try {
		SoLoud::BassboostFilter bass;
		if (a.empty()) {
			bass.setParams(0);
			soloud.setGlobalFilter(0, NULL);
			
		} else {
			bass.setParams(stof(a));
			soloud.setGlobalFilter(0, &bass);
		}
		
		

		return 0;
	} catch (...) {

	}
}

int reverb(string a) {
	SoLoud::FreeverbFilter revrb;
	if (a.empty()) {
		
		soloud.setGlobalFilter(1, NULL);
	} else {
		revrb.setParams(0, 0.5, 0.5, 1);
		soloud.setGlobalFilter(1, &revrb);
	}
	return 0;
}




const unordered_map<string, int (*)(string)> effects = {
	{"8D", Eff8D},
	{"bass", Bass},
	{"reverb", reverb}
};
int Effect(string Command) {
	if (Command.empty()) {
		cout << endl;
		for (auto& a : effects) {
			cout << a.first << endl;
		}

		cout << endl;
	} else {

		unsigned long long spacePos;
		for (int i = 0; i < Command.size() && Command[i] != ' '; i++)spacePos = i + 1;


		if (!(effects.find(Command.substr(0, spacePos)) == effects.end())) {
			string temp = Command.substr(0, spacePos);
			Command.erase(0, spacePos);
			if (!Command.empty()) {
				if (Command[0] == ' ') {
					Command.erase(0, 1);
				}
			}
			((effects.at(temp))(Command));
		}
	}

	return 0;
}

#pragma endregion


const unordered_map<string,string> ManDefinitions= {
	{"man", " -> manual page, duh"},
	{"help", " -> do you really wonder that... or are you just trolling?"},
	{ "shuffle", " -> shuffles the queue" },
	{"playlist", " -> lists all saved playlists\nplaylist save [playlist name] -> saves playlist to file so you can easily load it when you want to\nplaylist load [playlist name] -> loads playlist from file"},
	{"effect", " -> lists all effects\neffect [effect name] [parameter] -> activates desired effect"},
	{"pause"," -> you are definitelly trolling, IT PAUSES THE SONG"},
	{"unpause", " -> I give up, I hope you're proud of yourself. It does litterally the opposite of pause. IT UNPAUSES THE SONG"}
};



int man(string);


unordered_map<string, int (*)(string)> commands = {
	{"man", man},
	{"help", man},
	{"ping", ping},
	{"cd", move},
	{"exit", leave},
	{ "quit", leave },
	{ "leave", leave },
	{"close", leave},
	{ "dir", dir },
	{ "tree", tree },
	{ "play", play },
	{ "queue", Queue },
	{ "skip", skip },
	{ "prev", previous},
	{ "previous", previous},
	{ "back", previous},
	{ "volume", Volume },
	{ "v", Volume },
	{ "stop", stop },
	{ "shuffle", _Shuffle },
	{ "system", _System },
	{ "cls", _cls },
	{ "rotation", SoundRotation },
	{ "pan", SoundRotation },
	{ "playlist", Playlist },
	{ "playlists", Playlist },
	{ "effect", Effect},
	{ "pause", pause },
	{ "unpause", unpause },
	{"seek", seek},
	{"SetSong", SongGoTo},
	{"youtube", ytPlay},
	{"yt", ytPlay},
	{"ytPlay", ytPlay},
	{"ytplay", ytPlay}
};

int man(string a) {
	if (a.empty()) {

		cout << "list of commands:\n";
		for (auto& it : commands) {
			string b = it.first;
			cout << "\t" << (ManDefinitions.find(b) != ManDefinitions.end() ? "*|" : " |") << b << endl;
		}
	} else {
		cout << endl;
		if (ManDefinitions.find(a) != ManDefinitions.end())cout << "\n"<<a << ManDefinitions.at(a)<<endl;
		cout << endl;
	}

	return 0;
}


void Player() {

	
	soloud.init();
	while (true) {
		while (threadPause) {
			SoLoud::Thread::sleep(100);
			if (LeaveThreads) return;
		}
		//cout << SongQueue.empty();
		if (!SongQueue.empty() && soloud.getActiveVoiceCount() <= 0 && SongNUM < SongQueue.size()) {
			
			_sample.load(SongQueue[SongNUM].wstring().c_str());
			CurrentAudio = soloud.play(_sample);

			if (soloud.getActiveVoiceCount() > 0) {
#ifdef _WIN32
				SetConsoleTitle((L"Now Playing: " + SongQueue[SongNUM].wstring()).c_str());
#endif // _WIN32

				SongNUM++;
			}

			//CurrentSong = SongQueue[0];
			//SongQueue.erase(SongQueue.begin());
		} else if (SongNUM >= SongQueue.size()) {
			//SongNUM = 0;
			pause(string());
		}
		if (LeaveThreads) return;
		while (threadPause) {
			SoLoud::Thread::sleep(100);
			if (LeaveThreads) return;
		}
		while (soloud.getActiveVoiceCount() > 0) {
			if (LeaveThreads) return;
			SoLoud::Thread::sleep(100);
			while (threadPause) {
				SoLoud::Thread::sleep(100);
				if (LeaveThreads) return;
			}
		}
		while (threadPause) {
			SoLoud::Thread::sleep(100);
			if (LeaveThreads) return;
		}
		SoLoud::Thread::sleep(100);
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

		if (LeaveThreads) return;
		if (!is8DActive) return;
		SoLoud::Thread::sleep(timeout8D);
	}
	return;
}

int main(int argc, char* argv[]) {

	
	int spacePos;
	std::srand(unsigned(std::time(0)));
	string Command;

	
	//SongQueue.push_back("HailKingDN.mp3");

	cout << filesystem::absolute(Location) << ">>";
	getline(cin, Command);
	

	for (int i = 0; i < Command.size() && Command[i] != ' '; i++)spacePos = i+1;
	
	
		if (!(commands.find(Command.substr(0, spacePos)) == commands.end())) {
			string temp = Command.substr(0, spacePos);
			Command.erase(0, spacePos);
			if (!Command.empty()) {
				if (Command[0] == ' ') {
					Command.erase(0, 1);
				}
			}
			((commands.at(temp))(Command));
		}
	

	
	
	

		

	while (true) {
		cout << filesystem::absolute(Location) << ">>";
		getline(cin, Command);

		for (int i = 0; i < Command.size() && Command[i] != ' '; i++)spacePos = i + 1;

		/*for (int i = 0; i < sizeof(Commands) / sizeof(Commands[0]); i++) {
			if (Command.substr(0, spacePos) == Commands[i]) {
				Command.erase(0, spacePos);
				if (!Command.empty()) {
					if (Command[0] == ' ') {
						Command.erase(0, 1);
					}
				}
				(*CommandFunctions[i])(Command);
			}
		}*/

		if (!(commands.find(Command.substr(0, spacePos)) == commands.end())) {
			string temp = Command.substr(0, spacePos);
			Command.erase(0, spacePos);
			if (!Command.empty()) {
				if (Command[0] == ' ') {
					Command.erase(0, 1);
				}
			}
			((commands.at(temp))(Command));
		}
		
	}
	LeaveThreads = true;
	if (t1.joinable()) t1.join();
	if (loop.joinable()) loop.join();
	if (ytDlThrd.joinable())ytDlThrd.join();

	return 0;
}
