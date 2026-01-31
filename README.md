# ft_irc



# Missing
- #DONE setsocketopt before bind
- #DONE fcntl for non-blocking
- listen () after bind & poll()



# TODO
- Handle SIGINT and SIGQUIT signals


# commands
PRIVMSG, MODE
KICK, INVITE (kontrol), TOPIC


## Needs Impelementation
- kick.cpp 
- mode.cpp 
- notice.cpp -> yazmıyomuşuz aakyuz öyle dediiii 

quit'ten sonra command entry broken pipe veriyor nc kaynaklı olabilir

## Implemented
- topic.cpp 
- privmsg.cpp 
- invite.cpp
- join.cpp
- part.cpp
- nick.cpp 
- pass.cpp 
- quit.cpp 
- user.cpp