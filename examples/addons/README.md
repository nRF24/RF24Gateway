 **RF24Network LUA dissector for Wireshark**
 
 Usage: 
 1. Start wireshark on the host machine running RF24Gateway
 2. Use the command `wireshark -X lua_script:RF24Network.lua`
 3. In filters, add '!icmp' and apply
 4. RF24Network user payloads show under Protocol as RF24Net, and information is displayed