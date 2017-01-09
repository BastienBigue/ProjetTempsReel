/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tServeur;
extern RT_TASK tconnect;
extern RT_TASK tmove;
extern RT_TASK tenvoyer;
extern RT_TASK twatchdog;
extern RT_TASK tbattery;

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;

/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semConnectionMoniteur ; 
extern RT_SEM semConnectionOK ; 
/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @descripteurs des flags */
extern RT_EVENT ev_desc;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int nbSemaphore ; 

extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TWATCHDOG;
extern int PRIORITY_TBATTERY;	
#endif	/* GLOBAL_H */

