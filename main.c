/* 
 * File:   main.c
 * Author: Mikko Kosonen
 *
 * Created on June 30, 2021, 12:55 PM
 */

#define _XTAL_FREQ 4000000      // K‰ytett‰v‰n mikrokontrollerin kellotaajuus
#include <xc.h>                 // PIC-mikrokontrollerin kirjastot
#include <eeprom_routines.h>    // EEPROM-rutiinien kirjastot

#pragma config FOSC = EXTRC     // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF       // Watchdog Timer (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (Power-up Timer is disabled)
#pragma config CP = OFF         // Code Protection bit (Code protection disabled)

void NappiPainettu(void);       // Tarkistaa, onko nappia painettu
void Toiminnot();               // Suorittaa valitut toiminnot
void Lukitus(int lukittu);      // Lukitsee tai avaa lukituksen
void SalasananVaihto();         // Mahdollistaa salasanan vaihtamisen
void NapinTunnistus(int Rivi);  // Tunnistaa painetun napin

// Globaaleja muuttujia
int aloitettu = 0;            // Onko alustus suoritettu
int virrankierto = 1;         // N‰pp‰inten lukemisen tila
// int virtajalka;               // Virran tilan muuttuja
int kyseinennappi;            // Tunnistetun napin arvo

int painokerrat = 0;          // Painallusten m‰‰r‰
int tila;                     // Nykytila (lukittu(0)/avattu(1)/salasananvaihto(2))
int salasananpituus = 0;      // Salasanan pituus
char salasana[10];            // Tallennettu salasana
int kohta = 0;                // Seuraavan merkin indeksi

int merkki = 0;               // Uuden salasanan merkkien m‰‰r‰
char uusisalasana[10];        // Vaihdettava salasana

void main(void) {
    // Alustus suoritetaan vain kerran
    if (aloitettu == 0) {
        if (eeprom_read(12) != 1) {
            eeprom_write(0, 0);
            eeprom_write(11, 1);
            eeprom_write(12, 1);
        }
        // Asetetaan pinit OUtPUT tai INPUT
        TRISB = 0b00001111;
        TRISA = 0b10000;
        
        // Ladataan salasana EEPROM:sta
        for (int i = 0; i < 11; i++) {
            salasana[i] = eeprom_read(i);
        }
        salasananpituus = (int) eeprom_read(11);

        // Aluksi lukittu
        Lukitus(1);
        tila = 0;

        if (PORTAbits.RA4 == 1) {
            Lukitus(0);
        }
        aloitettu = 1;
    }

alku:
    PORTB = 0b00000000;
    virrankierto = 1;

    // N‰pp‰imistˆn skannaus: vuorotellen aktivoidaan jokainen sarake ja luetaan rivit
    while (virrankierto == 1) {
        PORTBbits.RB4 = 1;
        NappiPainettu();
        if (virrankierto == 0)
            goto alku;
        PORTBbits.RB4 = 0;

        PORTBbits.RB5 = 1;
        NappiPainettu();
        if (virrankierto == 0)
            goto alku;
        PORTBbits.RB5 = 0;

        PORTBbits.RB6 = 1;
        NappiPainettu();
        if (virrankierto == 0)
            goto alku;
        PORTBbits.RB6 = 0;

        PORTBbits.RB7 = 1;
        NappiPainettu();
        if (virrankierto == 0)
            goto alku;
        PORTBbits.RB7 = 0;
    }
}

// Tarkistaa, onko nappia painettu ja tunnistaa rivin
void NappiPainettu(void) {
    if (PORTBbits.RB0 || PORTBbits.RB1 || PORTBbits.RB2 || PORTBbits.RB3) {
        if (PORTBbits.RB0) {
            virrankierto = 0;
            NapinTunnistus(1);
            Toiminnot();
        } else if (PORTBbits.RB1) {
            virrankierto = 0;
            NapinTunnistus(2);
            Toiminnot();
        } else if (PORTBbits.RB2) {
            virrankierto = 0;
            NapinTunnistus(3);
            Toiminnot();
        } else if (PORTBbits.RB3) {
            virrankierto = 0;
            NapinTunnistus(4);
            Toiminnot();
        }
        
        // Jos tila on salasananvaihto, kutsutaan SalasananVaihtoa
        if (tila == 2) {
            PORTAbits.RA0 = 1;
            PORTAbits.RA1 = 1;
            SalasananVaihto();
        }
        
        // Vapautetaan kaikki sarakkeet
        PORTBbits.RB4 = 1;
        PORTBbits.RB5 = 1;
        PORTBbits.RB6 = 1;
        PORTBbits.RB7 = 1;
        
        // Odotetaan, kunnes nappi vapautetaan, jotta ei lueta heti samaa nappia uudestaan
        while (PORTBbits.RB0 == 1 || PORTBbits.RB1 == 1 || PORTBbits.RB2 == 1 || PORTBbits.RB3 == 1) {
        }
        __delay_ms(50);
    } else {
        virrankierto = 1;
    }
}

// Suorita toimintoja 
void Toiminnot() {
    
    // Siirry salasanan vaihtotilaan
    if (kyseinennappi == 11 && tila == 1) {
        tila = 2;
        PORTAbits.RA0 = 1;
        PORTAbits.RA1 = 1;
        
    // Salasanan tarkistus, kun kaappi lukittu. Jos v‰‰rin -> vilkuta punaista
    } else if (kyseinennappi == 11 && tila == 0) {
        if (kohta == salasananpituus && painokerrat == salasananpituus) {
            Lukitus(0);
        } else {
            kohta = 0;
            painokerrat = 0;
            for (int i = 0; i < 4; i++) {
                PORTAbits.RA0 = 1;
                PORTAbits.RA2 = 1;
                __delay_ms(60);
                PORTAbits.RA0 = 0;
                PORTAbits.RA2 = 0;
                __delay_ms(60);
            }
        }
    
    // Lukitaan
    } else if (kyseinennappi == 10 && tila != 2) {
        Lukitus(1);
    }
    
    // Ohjataan LED-valoja tilan mukaan
    if (tila == 0) {
        PORTAbits.RA0 = 0;
        __delay_ms(30);
        PORTAbits.RA0 = 1;
    } else if (tila == 1) {
        PORTAbits.RA1 = 0;
        __delay_ms(30);
        PORTAbits.RA1 = 1;
    } else if (tila == 2) {
        PORTAbits.RA0 = 0;
        PORTAbits.RA1 = 0;
        __delay_ms(30);
        PORTAbits.RA0 = 1;
        PORTAbits.RA1 = 1;
    }
    if (kyseinennappi == salasana[kohta]) {
        kohta++;
    }
    if (kyseinennappi != 11 && kyseinennappi != 10) {
        painokerrat++;
    }
}

// Vaihtaa lukituksen tilan
void Lukitus(int lukittu) {
    if (lukittu == 1) {
        kohta = 0;
        painokerrat = 0;
        PORTAbits.RA0 = 1;
        PORTAbits.RA1 = 0;
        PORTAbits.RA3 = 0;
        tila = 0;
    } else {
        PORTAbits.RA0 = 0;
        PORTAbits.RA1 = 1;
        PORTAbits.RA3 = 1;
        tila = 1;
    }
}

// Mahdollistaa salasanan vaihtamisen
void SalasananVaihto() {

    if (kyseinennappi != 11 && kyseinennappi != 10) {
        uusisalasana[merkki] = kyseinennappi;
        merkki++;
    }
    
    // Uuden salasanan tallennus EEPROM:iin
    if ((kyseinennappi == 11 && merkki > 0) || merkki == 10) {
        salasananpituus = merkki;
        for (int i = 0; i < 11; i++) {
            salasana[i] = uusisalasana[i];
            eeprom_write(i, uusisalasana[i]);
        }
        eeprom_write(11, salasananpituus);
        PORTAbits.RA0 = 0;
        tila = 1;
        merkki = 0;
    }
}

// Rivi luettu -> katsotaan mik‰ sarake p‰‰ll‰ ja p‰‰tell‰‰ nappi.
void NapinTunnistus(int Rivi) {

    if (Rivi == 1) {
        if (PORTBbits.RB4 == 1) {
            kyseinennappi = 1;

        } else if (PORTBbits.RB5 == 1) {
            kyseinennappi = 4;

        } else if (PORTBbits.RB6 == 1) {
            kyseinennappi = 7;

        } else if (PORTBbits.RB7 == 1) {
            kyseinennappi = 10;
        }
    }//--------------------------------//

    else if (Rivi == 2) {
        if (PORTBbits.RB4 == 1) {
            kyseinennappi = 2;

        } else if (PORTBbits.RB5 == 1) {
            kyseinennappi = 5;

        } else if (PORTBbits.RB6 == 1) {
            kyseinennappi = 8;

        } else if (PORTBbits.RB7 == 1) {
            kyseinennappi = 0;
        }
    }//--------------------------------//

    else if (Rivi == 3) {
        if (PORTBbits.RB4 == 1) {
            kyseinennappi = 3;

        } else if (PORTBbits.RB5 == 1) {
            kyseinennappi = 6;

        } else if (PORTBbits.RB6 == 1) {
            kyseinennappi = 9;

        } else if (PORTBbits.RB7 == 1) {
            kyseinennappi = 11;
        }
    }//--------------------------------//

    else if (Rivi == 4) {
        if (PORTBbits.RB4 == 1) {
            kyseinennappi = 12;

        } else if (PORTBbits.RB5 == 1) {
            kyseinennappi = 13;

        } else if (PORTBbits.RB6 == 1) {
            kyseinennappi = 14;

        } else if (PORTBbits.RB7 == 1) {
            kyseinennappi = 15;
        }
    }
}

