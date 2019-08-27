Tässä vaiheessa softarunkoa to 29.8. palaverin pohjaksi. 

Tausta-ajatuksia:

- Eri (sensorointi)tarkoituksiin voi tämän pohjalta tehdä käännöspaketteja, joihin valitaan mukana olevat taskit.
- yksittäinen taski voi olla esim. 
  - sensorointi: yhden sensorin (lämpö/kosteus, valo,...) luku globaaliin muistiin, 
  - lähetys: datapaketin muodostus ja lähetys (lora/wifi)
  - relay switch: kytketään esim. virtaa kuluttava laite pois, jos jännite alhaalla
  - ota-päivitys
- Kullekin taskille määritetään dynaamisesti seuraava ajoaika. Taski voi määrittää sen itsenäisesti (esim. seuraava ajo 600 sek kuluttua)
ja myös toinen taski voi muuttaa sitä (esim. pax-counterin onnistunut luku voi laittaa lora-lähetyksen heti jonoon)
- Loopissa taskeilla on ajojärjestys. Sensorointi alkuun ja sensoridataa hyödyntävät taskit lopuksi
- Loopin lopussa katsotaan mikä on seuraava taskin suoritusaika next_run_time[taskin_numero] ja jos ei heti perään, niin viedään deepsleepiin (huomioiden, että LoRa-viestit ehtivät lähteä ja tulla)
- Taskit lähtökohtaisesti synkronisesia eli blokkaavat muun suorituksen, mutta LoRa-kommunikaatio voi olla erikoistapaus...tämä täytyy miettiä
- Globaalit muuttujat, jotka säilytettävä sleeppien välillä pidetään RTC_DATA_ATTR 4k muistissa
- Olisi hyvä saada luettuja reaaliaika LoRan paluuviestillä ja/tai avoimen wifi-yhteyden yhteydessä 

- Koodi on ad-hoc ideoinnin pohjaksi. Esim. kaikkiin taskeihin liittyvä koodi viedään omiin onclude-fileihin, jotta runko pysyisi selkeänä

  
