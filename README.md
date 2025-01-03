# Electronic-safe

Sähköinen kassakaappi, jonka keskuksena C-kielellä ohjelmoitu PIC16F84A-mikrokontrolleri. Mahdollisuutena valita 1-10 merkkiä pitkä salasana, joka voidaan vaihtaa aina, kun kassakaappi on avattu. Salasanan tallennetaan EEPROM-muistiin, joten uusi salasana tulee pysymään käytössä, vaikka kassakaappi olisikin pitkään ilmaan sähköä.

#-näppäin on tarkoitettu vain salasanan hyväksymiseen, sekä aktivoimaan salasanan vaihtaminen. *-näppäin on kassakaapin lukitsemiseen. Se myös nollaa syötetyt merkit painettaessa kesken salasanan syöttämisen (lukituna). Näitä näppäimiä ei voida sisällyttää salasanaan.

Väärän salasanan tapauksessa rgb-ledi vilkuttaa punaista sekä soittaa summeria mikäli kytkin takana valittuna. Salasanan vaihdon aikana rgb-ledin on keltainen. Kun kassakaappi on auki, ledi on vihreä.

Lopullinen ohjelma on kehitetty MPLAB X IDE:llä. Konseptia kehittäessä käytin Arduinoa selkeyden ja piirin kestävyyden takia.
