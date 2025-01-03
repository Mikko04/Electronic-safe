/* 
 * File:   main.c
 * Author: Omistaja
 *
 * Created on June 30, 2021, 12:55 PM
 */

#define _XTAL_FREQ 4000000
#include <xc.h>
#include <eeprom_routines.h>

#pragma config FOSC = EXTRC     // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF        // Watchdog Timer (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (Power-up Timer is disabled)
#pragma config CP = OFF         // Code Protection bit (Code protection disabled)

void NappiPainettu(void);
void Toiminnot();
void Lukitus(int lukittu);
void SalasananVaihto();
void NapinTunnistus(int Rivi);

int aloitettu = 0;
int virrankierto = 1;
int virtajalka;
int kyseinennappi;

int painokerrat = 0;
int tila;
int salasananpituus = 0;
char salasana[10];
int kohta = 0;

int merkki = 0;
char uusisalasana[10];

void main(void) {

    if (aloitettu == 0) {
        if (eeprom_read(12) != 1) {
            eeprom_write(0, 0);
            eeprom_write(11, 1);
            eeprom_write(12, 1);
        }
        TRISB = 0b00001111;
        TRISA = 0b10000;
        for (int i = 0; i < 11; i++) {
            salasana[i] = eeprom_read(i);
        }
        salasananpituus = (int) eeprom_read(11);

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
        if (tila == 2) {
            PORTAbits.RA0 = 1;
            PORTAbits.RA1 = 1;
            SalasananVaihto();
        }
        PORTBbits.RB4 = 1;
        PORTBbits.RB5 = 1;
        PORTBbits.RB6 = 1;
        PORTBbits.RB7 = 1;
        while (PORTBbits.RB0 == 1 || PORTBbits.RB1 == 1 || PORTBbits.RB2 == 1 || PORTBbits.RB3 == 1) {
        }
        __delay_ms(50);
    } else {
        virrankierto = 1;
    }
}

void Toiminnot() {
    if (kyseinennappi == 11 && tila == 1) {
        tila = 2;
        PORTAbits.RA0 = 1;
        PORTAbits.RA1 = 1;
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
    } else if (kyseinennappi == 10 && tila != 2) {
        Lukitus(1);
    }
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

void SalasananVaihto() {

    if (kyseinennappi != 11 && kyseinennappi != 10) {
        uusisalasana[merkki] = kyseinennappi;
        merkki++;
    }
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

