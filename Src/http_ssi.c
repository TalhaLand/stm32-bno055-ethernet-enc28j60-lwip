/*
 * http_ssi.c
 */

#include "http_ssi.h"
#include "string.h"
#include "stdio.h"
#include "BNO055.h"
#include "lwip/tcp.h"
#include "lwip/apps/httpd.h"
#include "stm32f4xx_hal.h"

extern sensor_info_t sensor_info;
extern test_value test;

int indx = 0;

char const* TAGCHAR[] = {
    "x", "y", "z",
    "hx", "hy", "hz",
    "sx", "sy", "sz",
    "calstat", "samples"
};

char const** TAGS = TAGCHAR;

uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen)
{
    switch (iIndex)
    {
        case 0:
            sprintf(pcInsert, "%f", sensor_info.roll);
            return strlen(pcInsert);

        case 1:
            sprintf(pcInsert, "%f", sensor_info.pitch);
            return strlen(pcInsert);

        case 2:
            sprintf(pcInsert, "%f\t", sensor_info.yaw);
            return strlen(pcInsert);

        case 3:
            sprintf(pcInsert, "%f", sensor_info.mag_hard_iron[0]);
            return strlen(pcInsert);

        case 4:
            sprintf(pcInsert, "%f", sensor_info.mag_hard_iron[1]);
            return strlen(pcInsert);

        case 5:
            sprintf(pcInsert, "%f", sensor_info.mag_hard_iron[2]);
            return strlen(pcInsert);

        case 6:
            sprintf(pcInsert, "%f", sensor_info.mag_soft_iron[0]);
            return strlen(pcInsert);

        case 7:
            sprintf(pcInsert, "%f", sensor_info.mag_soft_iron[1]);
            return strlen(pcInsert);

        case 8:
            sprintf(pcInsert, "%f", sensor_info.mag_soft_iron[2]);
            return strlen(pcInsert);

        case 9: // <!--#calstat-->
            if (sensor_info.calibration_requested == 1)
                sprintf(pcInsert, "Kalibrasyon Yapiliyor...");
            else if (sensor_info.calibration_done == 1)
                sprintf(pcInsert, "Tamamlandi");
            else
                sprintf(pcInsert, "Hazir");
            return strlen(pcInsert);

        case 10: // <!--#samples-->
            sprintf(pcInsert, "%u", test.ornek_sayi2);
            return strlen(pcInsert);

        default:
            break;
    }

    return 0;
}

const char *cgi_form(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

const tCGI form_cgi = {"/calibrate.cgi", cgi_form};

const char *cgi_form(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    if (iIndex == 0)
    {
        for (int i = 0; i < iNumParams; i++)
        {
            if (strcmp(pcParam[i], "start") == 0 && strcmp(pcValue[i], "1") == 0)
            {
                sensor_info.calibration_requested = 1;
            }
        }
    }

    return "/calibration.shtml";
}

void http_server_init(void)
{
    httpd_init();

    http_set_ssi_handler(ssi_handler, (char const**) TAGS, 11);

    http_set_cgi_handlers(&form_cgi, 1);
}
