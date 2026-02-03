### Nasıl Çalıştıracaksın?

1. **Önce Sunucunu Başlat:**
Terminal 1: `./ircserv 6667 1234`
2. **Botu Derle ve Başlat:**
Terminal 2:
```bash
c++ bot.cpp -o bot
./bot

```


(Botun içindeki `BOT_PASS` ile sunucunun şifresinin aynı olduğundan emin ol).
3. **Test Et:**
Terminal 3 (HexChat veya nc): Sunucuya bağlan.
* `/join #sohbet`
* Bot zaten oraya girmemiş olabilir, sen botu çağırabilirsin veya kodun içine `sendRaw(sock, "JOIN #sohbet");` ekleyebilirsin.
* Ancak en kolayı, bota özel mesaj at: `/msg SelamBot Selam`
* Veya botla aynı kanaldaysan kanala `Selam` yaz.



### Bot Mantığı Nasıl Çalışıyor?

1. **External Client:** Bot aslında sadece yetenekleri kısıtlı bir `Client`'tır. Sunucun onu gerçek bir insan sanır.
2. **PING/PONG:** Sunucu belirli aralıklarla `PING` atar. Eğer bot `PONG` cevabı vermezse sunucu "Bu düştü" diyip bağlantıyı keser. Kodda bunu hallettik.
3. **Parsing:** Gelen mesaj şöyledir: `:Ahmet!user@host PRIVMSG #kanal :Selam naber`.
* Kodumuz `PRIVMSG` kelimesini arar.
* `:` işaretinden sonraki mesaj içeriğini alır.
* İçinde "Selam" geçiyorsa, `sendRaw` ile `PRIVMSG #kanal :Merhaba...` paketini oluşturup sunucuya geri yollar.



Bu kadar! Bu basit kod parçası sana bonus puanı getirecektir. Sunucu kodunu kirletmeden temiz bir çözüm sunmuş olursun. Bol şans! 🚀