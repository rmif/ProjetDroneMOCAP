/****************************************************************************
 *
 *   Copyright (c) 2012-2016 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file pose.cpp
 * Projet Drone MOCAP pour l'Ecole Centrale de Nantes
 * Asservissement en position d'un drone quadcopter dans une cage
 * @author Paul Asquin <paul.asquin@gmail.com>
 */

#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_shutdown.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <systemlib/systemlib.h>
#include <systemlib/err.h>


static bool thread_should_exit = false;     /**< daemon exit flag */
static bool thread_running = false;     /**< daemon status flag */
static int pose_thread_task;             /**< Handle of daemon task / thread */

extern "C" __EXPORT int pose_main(int argc, char *argv[]);

int pose_thread_main(int argc, char *argv[]);

static void usage(const char *reason);

static void
usage(const char *reason)
{
    if (reason) {
        warnx("%s\n", reason);
    }

    warnx("pose : {start|stop|status} [-t minutesDeLog ] [-n (empeche reboot)]\n\n");
}

int pose_main(int argc, char *argv[])
{
    if (argc < 2) {
        usage("Erreur de commande");
        return 1;
    }

    if (!strcmp(argv[1], "start")) 
    {
        if (thread_running) {
                warnx("pose deja en cours\n");
                /* this is not an error */
                return 0;
        }

        thread_should_exit = false;
        pose_thread_task = px4_task_spawn_cmd("pose",
                         SCHED_DEFAULT,
                         SCHED_PRIORITY_DEFAULT,
                         7000,
                         pose_thread_main,
                         (argv) ? (char *const *)&argv[2] : (char *const *)NULL);//Appel d'un thread exécutant la fonction pose_thread_main avec envoie des paramètres
        return 0;
    }

    if (!strcmp(argv[1], "stop")) {
        if (!thread_running)
        {
            PX4_INFO("Thread pose pas en cours");
        }
        
        if (thread_running)// Si on demande un stop et que le thread est en cours, on annule le timer de redémarrage et d'annulation du log
        {
            PX4_INFO("Meurtre du tread");
            px4_task_delete(pose_thread_task);
            thread_running = false;
        }
        thread_should_exit = true;
        return 0;
    }

    if (!strcmp(argv[1], "status")) {//Afichage des statuts de l'application
        if (thread_running) {
            warnx("\tEn cours\n");

        } else {
            warnx("\tNon demarre\n");
        }

        return 0;
    }

    usage("Commande non reconnue");
    return 1;
}

int pose_thread_main(int argc, char *argv[])
{
    int min = 60;
    int reboot = true;
    thread_running = true;

    PX4_INFO("Demarrage pose_thread\n");

    if(!thread_should_exit) 
    {
        for (int i = 0; i < argc; ++i)//Lecture des paramètres données à l'application
        {
            if (!strcmp(argv[i], "-t"))//Si on lit le paramètres -t
            {
                if (argc >= i+1 && atoi(argv[i+1]) != 0)//On vérifie que l'on trouve encore des données après -t. Si atoi retourne 0, la valeur n'est pas un chiffre, ou 0....
                {
                    min = atoi(argv[i+1]);//On récupère le nombre de minutes à attendre
                }
                else
                {
                    warnx("Pas de paramètres valide après -t. On prend 0");
                    min = 0;
                }
                
            }

            if(!strcmp(argv[i], "-n"))//Si on lit le paramèter -n
            {
            
            }
        }
    }
    
    thread_running = false;
    return 0;
}
