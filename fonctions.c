#include "fonctions.h"
#define EVENT_WAIT_MASK   (0x1)

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void watchdog(void * arg);

void envoyer(void * arg) {
    DMessage *msg;
    int err;

    while (1) {
        rt_printf("tenvoyer : Attente d'un message\n");
        if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
            rt_printf("tenvoyer : envoi d'un message au moniteur\n");
            serveur->send(serveur, msg);
            msg->free(msg);
        } else {
            rt_printf("Error msg queue write: %s\n", strerror(-err));
        }
    }
}

void connecter(void * arg) { //ok
    int status;
    int err;
    DMessage *message;
    int i ; 

    rt_printf("tconnect : Debut de l'exécution de tconnect\n");

    while (1) {
        rt_printf("tconnect : Attente du sémarphore semConnecterRobot\n");
        rt_sem_p(&semConnecterRobot, TM_INFINITE);
        rt_printf("tconnect : Ouverture de la communication avec le robot\n");
        
        status = robot->open_device(robot);
        
		if (status == STATUS_OK) {
			status = robot->start_insecurely(robot);

				if (status == STATUS_OK) {
					rt_printf("tconnect : Lancement du robot\n");
					/*while (i < nbSemaphore) {
						rt_sem_v(&semConnectionOK) ; 
						i++ ;  */
					if (rt_event_signal(&ev_desc,(0x1)))
						{
							rt_printf("Envoi du flag impossible : arrêt du programme\n");
							exit(-1);
						}
					}
				
				else {
					rt_printf("tconnect : Erreur dans robot_start\n");
				}
			}
			
		else {
			rt_printf("tconnect : Erreur dans open_device \n");
		} 
			 

	   
        message = d_new_message();
        message->put_state(message, status);
        rt_printf("tconnecter : Connexion ok\n");
        message->print(message, 100);

        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }
    }
}

void communiquer(void *arg) { //ok
    DMessage *msg = d_new_message();
    int status = 1;
    int num_msg = 0;


	while(1) {
    rt_printf("tserve error: ‘semConnexr : Début de l'exécution de serveur\n");
    serveur->open(serveur, "8000");
    rt_printf("tserver : Connexion\n");

    /*rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommMoniteur = 0;
    rt_mutex_release(&mutexEtat);*/
    
    rt_sem_v(&semConnectionMoniteur) ; 
    rt_printf("tserver : Connexion avec le moniteur etablie\n");

	    while (status > 0) {
		   rt_printf("tserver : Attente d'un message\n");
		   status = serveur->receive(serveur, msg);
		   
		   
		   if (status > 0) {
			num_msg++;
			/* Rajouter dans ce switch les actions pour lancer la detection de l'arene, compute position,...*/ 
		       switch (msg->get_type(msg)) {
		           case MESSAGE_TYPE_ACTION:
		               rt_printf("tserver : Le message %d reçu est une action\n", num_msg);
		               DAction *action = d_new_action();
		               action->from_message(action, msg);
		               switch (action->get_order(action)) {
		                   case ACTION_CONNECT_ROBOT:
		                       rt_printf("tserver : Action connecter robot\n");
		                       rt_sem_v(&semConnecterRobot);
		                       break;
		               }
		               break;
		           case MESSAGE_TYPE_MOVEMENT:
		               rt_printf("tserver : Le message reçu %d est un mouvement\n", num_msg);
		               rt_mutex_acquire(&mutexMove, TM_INFINITE);
		               move->from_message(move, msg);
		               move->print(move);
		               rt_mutex_release(&mutexMove);
		               break;
		       }
		}
		else {
				rt_printf("tserver : Connexion avec le moniteur rompue\n");
			   rt_sem_p(&semConnectionMoniteur, TM_INFINITE) ; 
			   serveur->close(serveur) ; 
		}
	    }
    }
}


void battery (void* arg){ //task battery
	int status;
	DMessage *message ; 
	DBattery * sbattery = d_new_battery();
	
	unsigned long mask_ret;
	int vbat ;
	int compteur; //à remplacer par la variable partagée compteur
	rt_printf("tbattery : Debut de l'éxecution de periodique à 250ms\n");
	rt_task_set_periodic(NULL, TM_NOW, 250000000); //périodique de 250ms
	while(1) {
	
	    	rt_task_wait_period(NULL); 
	    	rt_event_wait(&ev_desc,EVENT_WAIT_MASK,&mask_ret,EV_ANY,TM_INFINITE);
	   	status = robot->get_vbat(robot , (&vbat));
		rt_printf("tbattery : status = %d\n", status) ; 
        	
        	if (status == STATUS_OK) {
        	 	compteur=0;
            	sbattery->set_level(sbattery, vbat);
            	message = d_new_message();
	          message->put_battery_level(message,sbattery);
	          message->print(message, 100) ; 
            	rt_printf("tbattery : Envoi message\n");
               
               if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
		 		message->free(message);
		  	}           
         }
       else {
       	compteur ++ ;
       }
  }
}     


void deplacer(void *arg) {
    int status = 1;
    int gauche;
    int droite;
    int lent ; 
    DMessage *message;
	unsigned long mask_ret;
    rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);

    while (1) {
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_event_wait(&ev_desc,EVENT_WAIT_MASK,&mask_ret,EV_ANY,TM_INFINITE);
        rt_printf("tmove : Activation périodique\n");
        
        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommRobot;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            rt_mutex_acquire(&mutexMove, TM_INFINITE);
            if (move->get_speed(move)<50) {
            	lent = 1 ; 
            } 
            else {
            	lent = 0 ; 
            } 
            
           move->print(move);
            switch (move->get_direction(move)) {
                case DIRECTION_FORWARD:
                	/*if (lent) {*/
		               gauche = MOTEUR_ARRIERE_LENT;
		               droite = MOTEUR_ARRIERE_LENT;
                	/*}
                	else {
		               gauche = MOTEUR_ARRIERE_RAPIDE;
		               droite = MOTEUR_ARRIERE_RAPIDE;
                    }*/
                    break;
                case DIRECTION_LEFT:
                  /* 	if (lent) {*/
		               gauche = MOTEUR_ARRIERE_LENT;
		               droite = MOTEUR_AVANT_LENT;
                	/*}
                	else {
		               gauche = MOTEUR_ARRIERE_RAPIDE;
		               droite = MOTEUR_AVANT_RAPIDE;
		              }*/
                    break;
                case DIRECTION_RIGHT:
                   	/*if (lent) {*/
		               gauche = MOTEUR_AVANT_LENT;
		               droite = MOTEUR_ARRIERE_LENT;
                	/*}
                	else {              
		               gauche = MOTEUR_AVANT_RAPIDE;
		               droite = MOTEUR_ARRIERE_RAPIDE;
                    }*/
                    break;
                case DIRECTION_STOP:
                    gauche = MOTEUR_STOP;
                    droite = MOTEUR_STOP;
                    break;
                case DIRECTION_STRAIGHT:
                	/*if (lent) {*/
		           	gauche = MOTEUR_AVANT_LENT;
		               droite = MOTEUR_AVANT_LENT;
                	/*}
                	else  {
		               gauche = MOTEUR_AVANT_RAPIDE;
		               droite = MOTEUR_AVANT_RAPIDE;
                    }*/
                    break;
            }
            rt_mutex_release(&mutexMove);

		  int nb_fail = 0;
		  while (nb_fail < 3) {
		       status = robot->set_motors(robot, gauche, droite);
		       if (status == STATUS_OK) { nb_fail = 0; break;}
		       else nb_fail++;
		  }
		       if (status != STATUS_OK) {
		           rt_mutex_acquire(&mutexEtat, TM_INFINITE);
		           //etatCommRobot = status;
		           rt_mutex_release(&mutexEtat);

		           message = d_new_message();
		           message->put_state(message, status);

		           rt_printf("tmove : Envoi message\n");
		           if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
		               message->free(message);
		           }
		       }
            
        }
    }
}

void watchdog(void * arg) {
	int status;
	DMessage *message;
	unsigned long mask_ret;
	
	
	rt_printf("twatchdog : Debut de l'éxecution de periodique à 1s\n");
    	rt_task_set_periodic(NULL, TM_NOW, 950000000);
	 
    	 while (1) {
        /* Attente de l'activation périodique */
        rt_event_wait(&ev_desc,EVENT_WAIT_MASK,&mask_ret,EV_ANY,TM_INFINITE);
        rt_printf("twatchdog : Activation périodique\n");

        /*rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommRobot;
        rt_mutex_release(&mutexEtat);
        rt_printf("status avant rentrer watchdog:%d\n",status);
        if (status == STATUS_OK) {
			
			int nb_fail = 0;
		  		while (nb_fail < 3) {
		       		status = robot->reload_wdt(robot);
		       		if (status == STATUS_OK) { 
				  		rt_printf("TWATCHDOG : RELOAD ENVOYE\n"); 
				  		nb_fail = 0; break;}
		       		else {
		       		rt_printf("status not OK : %d\n", status);
		       		nb_fail++;
		       		}
		  		}
		       if (status != STATUS_OK) {
		           rt_mutex_acquire(&mutexEtat, TM_INFINITE);
		           //etatCommRobot = status;
		           rt_mutex_release(&mutexEtat);

		           message = d_new_message();
		           message->put_state(message, status);

		           rt_printf("twatchdog : Envoi message\n");
		           if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
		               message->free(message);
		           }
		       }
		       
            
        }*/
        /*rt_sem_v(&semConnectionOK) ;*/
        rt_task_wait_period(NULL);
        
    }
	
}


int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
    void *msg;
    int err;

    msg = rt_queue_alloc(msgQueue, size);
    memcpy(msg, &data, size);

    if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
        rt_printf("Error msg queue send: %s\n", strerror(-err));
    }
    rt_queue_free(&queueMsgGUI, msg);

    return err;
}
