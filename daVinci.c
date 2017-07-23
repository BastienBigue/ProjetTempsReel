
#define EVENT_WAIT_MASK   (0x1)

/* GLOBAL*/

/* PRIORITY */
int PRIORITY_TCONTROLER = 60;
int PRIORITY_TCAPTURE = 80;
int PRIORITY_TDISPLAY = 70;


/* Structures globales */
DCommande * commande ; 
DImage * image ; 
DOSD * OSDbuffer ; 
DOSD * OSD ; 

/* MUTEX */
RT_MUTEX mutex_commande ; 
RT_MUTEX mutex_image ; 
RT_MUTEX mutex_osdbuffer ; 
RT_MUTEX mutex_osd ; 
RT_MUTEX mutex_ecran ; 

/* THREADS */
RT_TASK tcontroler ; 
RT_TASK tcapture ; 
RT_TASK tdisplay ; 


/* QUEUE */
RT_QUEUE queueMsgGUI;
int MSG_QUEUE_SIZE = 10;

/* EVENT FLAG */ 
RT_EVENT ev_desc;

/* PROTOTYPES */
void controler(void * arg) ; 
void capture(void * arg) ; 
void display(void * arg) ; 





/* FONCTIONS */

void controler(void * arg) {

	DOSD * osd ; 
	DCommande * nouvelle_commande ; 

	initialiser_le_système() ; 
	if (rt_event_signal(&ev_desc,(0x1))) {
		rt_printf("Envoi du flag impossible : arrêt du programme\n");
		exit(-1);
	}

	rt_task_set_periodic(NULL, TM_NOW, 80000000); 

	while(1) {

		rt_mutex_acquire(&mutex_commande) ; 
		nouvelle_commande = commande ; 
		rt_mutex_release(&mutex_commande) ; 

		osd = calcul_osd(nouvelle_commande) ; 

		rt_mutex_acquire(&mutex_osdbuffer) ; 
		OSDbuffer = osd ; 
		rt_mutex_release(&mutex_osdbuffer) ; 

		rt_mutex_acquire(&mutex_osd) ; 
		OSD = osd ; 
		rt_mutex_release(&mutex_osd) ; 

		rt_task_wait_period(NULL); 
	}


}


void capture(void * arg) {

	DImage * image_capturee ; 
	DMessage * message ; 

	rt_event_wait(&ev_desc,EVENT_WAIT_MASK,&mask_ret,EV_ANY,TM_INFINITE);

	while(1) {

		//rt_mutex_acquire(&mutex_image) ; 
		image_capturee = acquisition_image(image) ; 
		//Je suppose ici que acquisition_image utilise des mutex pour protéger l'accès a image.
		//rt_mutex_release(&mutex_image) ; 

		message = d_new_message();
		message->put_image(message, image_capturee);
		if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
			message->free(message);
		} 
	}
}


void display(void * arg) {

	DMessage * message ; 
	DOSD * osd ; 

	rt_event_wait(&ev_desc,EVENT_WAIT_MASK,&mask_ret,EV_ANY,TM_INFINITE);

	while(1) {

		rt_queue_read(&queueMsgGUI, &image_capturee, sizeof (DMessage), TM_INFINITE) ; 

		rt_mutex_acquire(&mutex_osd) ; 
		osd = OSD ; 
		rt_mutex_release(&mutex_osd) ; 

		img = encode(image_capturee, osd) ;

		rt_mutex_acquire(&mutex_ecran) ; 
		buffer_ecran = img ; 
		rt_mutex_release(&mutex_ecran) ; 
	}

}


/* MAIN */
int main(int argc, char ** argv) {
int err ; 

	/* Creation de l'event flag */
	if (err = rt_event_create(&ev_desc,"MyEventFlagGroup", 0x0 , EV_PRIO))
	{
		rt_printf("Error event flag create:\n");
		exit(-1);
	}

	/* Creation des mutex */
	if (err = rt_mutex_create(&mutex_osd, NULL)) {
		rt_printf("Error mutex create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_mutex_create(&mutex_osdbuffer, NULL)) {
		rt_printf("Error mutex create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_mutex_create(&mutex_ecran, NULL)) {
		rt_printf("Error mutex create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_mutex_create(&mutex_ecran, NULL)) {
		rt_printf("Error mutex create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_mutex_create(&mutex_image, NULL)) {
		rt_printf("Error mutex create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}

	/* Creation des taches */
	if (err = rt_task_create(&tcontroler, NULL, 0, PRIORITY_TCONTROLER, 0)) {
		rt_printf("Error task create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_task_create(&tcapture, NULL, 0, PRIORITY_TCAPTURE, 0)) {
		rt_printf("Error task create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_task_create(&tdisplay, NULL, 0, PRIORITY_TDISPLAY, 0)) {
		rt_printf("Error task create: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}


	/* Lancement des taches */
	if (err = rt_task_start(&tcontroler, &controler, NULL)) {
		rt_printf("Error task start: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_task_start(&tcapture, &capture, NULL)) {
		rt_printf("Error task start: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}
	if (err = rt_task_start(&tdisplay, &display, NULL)) {
		rt_printf("Error task start: %s\n", strerror(-err));
		exit(EXIT_FAILURE);
	}


	pause();

	/* Suppression des taches */
	rt_task_delete(&tServeur);
	rt_task_delete(&tconnect);
	rt_task_delete(&t);

	return 0 ; 
}