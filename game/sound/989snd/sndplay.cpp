#include <iostream>
#include <sstream>

#include "common/util/FileUtil.h"

#ifdef _WIN32
#include <windows.h>
#define sleep(n) Sleep(n * 1000)
#endif
#include "player.h"
#include <map>
#include "common/log/log.h"

int main(int argc, char* argv[]) {
  snd::Player player;
  std::map<std::string_view, snd::BankHandle> bankNameHandleMap;
  if(argc > 1){
    fs::path file = argv[1];
    auto file_buf = file_util::read_binary_file(file);
    auto bankid = player.LoadBank(file_buf);
    auto name = bankid->GetName()->empty() ? "default" : bankid->GetName().value();
    bankNameHandleMap.insert(std::make_pair(name, bankid));
    std::cout << "Soundbank " << name << " loaded\n";
    

    if (argc > 2) {
      unsigned sound = player.PlaySound(bankid, atoi(argv[2]), 0x400, 0, 0, 0);
      lg::info("sound {} started", sound);
    }
  }

  printf("commands:\n");
  printf(" play [bank-id] [sound-id]\n");
  printf(" load [path-to-file]\n");
  printf(" pause [sound-handle]\n");
  printf(" unpause [sound-handle]\n");
  printf(" stop\n");
  printf(" dump-info [bank-id]\n");
  printf(" setreg [sound-handle] [reg] [val]\n");
  printf(" exit\n");


  bool exit = false;
  while (!exit) {
    printf("> ");
    std::string command;
    std::getline(std::cin, command);

    std::stringstream ss(command);
    std::string tmp;
    std::vector<std::string> parts;
    while (std::getline(ss, tmp, ' ')) {
      parts.push_back(tmp);
    }
    if(parts.size() == 0){
      continue;
    }

    if (parts[0] == "play"){
      if (parts.size() < 3 || !bankNameHandleMap.contains(parts[1])){
        printf("invalid args\n");
      } else {
        auto id = player.PlaySound(bankNameHandleMap[parts[1]], std::atoi(parts[2].c_str()), 0x400, 0, 0, 0);
        //TODO:: should print no sound handle if player.playsound doesn't work
        printf("sound handle %d started\n", id);
      }
    }
    else if(parts[0] == "load" && parts.size() == 2){
      fs::path file = parts[1];
      auto file_buf = file_util::read_binary_file(file);
      auto bankid = player.LoadBank(file_buf);
      auto name = bankid->GetName()->empty() ? "default" : bankid->GetName().value();
      bankNameHandleMap.insert(std::make_pair(name, bankid));
      std::cout << "Soundbank " << name << " loaded\n";
    }
    else if(parts[0] == "pause" && parts.size() == 2){
      player.PauseSound(std::atoi(parts[1].c_str()));
    }
    else if(parts[0] == "unpause" && parts.size() == 2){
      player.ContinueSound(std::atoi(parts[1].c_str()));
    }
    //TODO: this option sucks
    // else if (parts[0] == "playall") {
    //   auto idx = 0;
    //   auto id = player.PlaySound(bankid, idx, 0x400, 0, 0, 0);
    //   while (true) {
    //     if (player.SoundStillActive(id)) {
    //       sleep(1);
    //     } else {
    //       idx++;
    //       id = player.PlaySound(bankid, idx, 0x400, 0, 0, 0);
    //     }
    //   }
    // }
    else if (parts[0] == "setreg") {
      if (parts.size() < 3) {
        printf("invalid args\n");
      } else {
        player.SetSoundReg(std::atoi(parts[1].c_str()), std::atoi(parts[2].c_str()),
                           std::atoi(parts[3].c_str()));
      }
    }
    else if (parts[0] == "stop") {
      printf("stopping all sounds\n");
      player.StopAllSounds();
    }
    else if (parts[0] == "dump-info" && parts.size() == 2 && bankNameHandleMap.contains(parts[1])){
      player.DebugPrintAllSoundsInBank(bankNameHandleMap[parts[1]]);
    }
    else if (parts[0] == "exit"){
      exit = true;
    }
    else{
      printf("Did not recognize your command, please try again.\n");
    }
  }

  return 0;
}
