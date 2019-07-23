#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/stb_json.hpp"

#define KB 1024
#define MB 1048576

int main()
{

    printf("Testing C++ wrapper to stb_json\n");

    char buf[3*MB];
    {
        // Huge JSON test ---------------------
        char str[100];

        printf("READING CANADA.JSON ---------------------\n");
        FILE* fp = fopen("canada.json", "rb");
        if(fp)
        {
            unsigned int len = fread(buf, 1, 3*MB, fp);
            printf("canada.json opened correctly, size %i\n", len);
            fclose(fp);

            stb_json json(buf, len);
            int count = json.Count();
            printf("num element from root: %i\n", count);

            if(json.HasError()) 
                printf("ERROR: %s\n", json.GetError());

            json.GetString("type", str, 100);

            printf("Type : %s\n", str);

            json = json.MoveCursor("features");
            json = json.MoveCursor(0);

            if(json.HasError()) 
                printf("ERROR: %s\n", json.GetError());

            json.GetString("type", str, 100);
            printf("-type : %s\n", str);

            json = json.MoveCursor("geometry");
            json = json.MoveCursor("coordinates");

            if(json.HasError()) 
                printf("ERROR: %s\n", json.GetError());

            count = json.Count();

            for(int a=0; a < count; ++a)
            {
                stb_json json2 = json.MoveCursor(a);
                int count2 = json2.Count();

                printf("Shape %i contains %i coords\n", a, count2);

                for(int b=0; b < count2; ++b)
                {
                    stb_json json3 = json2.MoveCursor(b);
                    printf(">>> Coord %i: %f %f\n", b, 
                            json3.GetDouble(0), json3.GetDouble(1));
                }
            }
        }
        else
            printf("Could not open canada.json\n");
    }

    {
        // Huge JSON test 2 ---------------------
        char str[100];

        printf("READING CITM_CATALOG.JSON ---------------------\n");
        FILE* fp = fopen("citm_catalog.json", "rb");
        if(fp)
        {
            unsigned int len = fread(buf, 1, 2*MB, fp);
            printf("citm_catalog.json opened correctly, size %i\n", len);
            fclose(fp);

            stb_json json(buf, len);
            int count = json.Count();
            printf("Num element from root: %i\n", count);

            if(json.HasError()) 
                printf("ERROR: %s\n", json.GetError());

            json = json.MoveCursor("events");
            count = json.Count();
            printf("Num element on events: %i\n", count);

            if(json.HasError()) 
                printf("ERROR: %s\n", json.GetError());

            for(int a=0; a < count; ++a)
            {
                stb_json json2 = json.MoveCursor(a);

                json2.GetString("name", str, 100);
                printf("Event %i -----------\nName: %s\n", a, str);

                json2.GetString("description", str, 100);
                printf("Description: %s\n", str);

                json2.GetString("logo", str, 100);
                printf("Logo: %s\n", str);

                printf("Id: %i\n", json2.GetInt("id"));

                if(json.HasError()) 
                    printf("ERROR: %s\n", json.GetError());
            }
        }
        else
            printf("Could not open citm_catalog.json\n");
    }
}

