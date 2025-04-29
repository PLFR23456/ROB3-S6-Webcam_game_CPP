#include <stdlib.h>
#include <gtk/gtk.h>


// Déclaration des fonctions
void on_activate_entry(GtkWidget *pEntry, gpointer data);
void on_copier_button(GtkWidget *pButton, gpointer data);
 
 
int main(int argc,char **argv)
{

    GtkWidget* pWindow;
    GtkWidget *pVBox;
    GtkWidget* pLabel;
    GtkWidget *pButton;
    GtkWidget *pEntry;
    GtkWidget *pResult;
    GtkWidget *pImage;
    
    // Initialisation de GTK
    gtk_init(&argc,&argv);
 
    // Création de la fenetre principale (titre et taille de la fenetre)
    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pWindow), "Exemples GTK");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 320, 200);
    
    // Connexion du signal de fermeture de la fenetre au fait de quitter le programme
    g_signal_connect(G_OBJECT(pWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);
 
 	
    // Creation d'une verticale box qu'on ajoute à la fenetre principale
    // Attention ! La fenetre principale GTK ne peut contenir qu'un seul widget
    // Il faut ensuite utiliser des vbox et des hbox pour positionner les widgets à l'intérieur
    pVBox = gtk_vbox_new(TRUE, 0);    
    gtk_container_add(GTK_CONTAINER(pWindow), pVBox);
    
    
    /* Creation des labels */
    // Attention, le text du label doit etre converti en utf-8 si il comporte des caractères accentués, en utilisant : g_locale_to_utf8()
    pLabel=gtk_label_new("Label :");
    pResult = gtk_label_new(" ");
    
    /* Creation du GtkEntry */
    pEntry = gtk_entry_new();
    
    /*Creation du bouton */
    pButton = gtk_button_new_with_label("Bouton 1");
    
    /* Creation de l'image */
    pImage = gtk_image_new_from_file("polytech.jpg");
 
    
    /* Insertion des widgets dans la GtkVBox */
    // L'ordre d'insertion des éléments dans la vBox correspond à l'ordre d'apparition des éléments dans la fenetre
    gtk_box_pack_start(GTK_BOX(pVBox), pLabel, TRUE, FALSE, 0);   
    gtk_box_pack_start(GTK_BOX(pVBox), pEntry, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButton, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pResult, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pImage, FALSE, FALSE, 5);
    
    /* Connexion du signal "activate" du GtkEntry */
    g_signal_connect(G_OBJECT(pEntry), "activate", G_CALLBACK(on_activate_entry), (GtkWidget*) pLabel);
 
    /* Connexion du signal "clicked" du GtkButton */
    g_signal_connect(G_OBJECT(pButton), "clicked", G_CALLBACK(on_copier_button), (GtkWidget*) pVBox);
 
 
    /* Affichage de la fenêtré et de tout ce qu'il contient */
    gtk_widget_show_all(pWindow);
 
   
    /* Demarrage de la boucle évenementielle */
    gtk_main();
 
    return EXIT_SUCCESS;
}

/* Fonction callback execute lors du signal "activate" */
void on_activate_entry(GtkWidget *pEntry, gpointer data)
{
    const gchar *sText;
 
    /* Recuperation du texte contenu dans le GtkEntry */
    sText = gtk_entry_get_text(GTK_ENTRY(pEntry));
 
    /* Modification du texte contenu dans le GtkLabel */
    gtk_label_set_text(GTK_LABEL((GtkWidget*)data), sText);
}

/* Fonction callback executee lors du signal "clicked" */
void on_copier_button(GtkWidget *pButton, gpointer data)
{
    GtkWidget *pTempEntry;
    GtkWidget *pTempLabel;
    GList *pList;
    const gchar *sText;
 
    /* Récupération de la liste des éléments que contient la GtkVBox */
    // Le premier élement de la GList est le Label
    pList = gtk_container_get_children(GTK_CONTAINER((GtkWidget*)data));


 
    /* Passage à l élément suivant : le GtkEntry */
    pList = g_list_next(pList);
    /* Récupération du GtkEntry */
    pTempEntry = GTK_WIDGET(pList->data);
    
    /* Passage à l élément suivant : le GtkButton */
    pList = g_list_next(pList);
 
    /* Passage à l élément suivant : le GtkLabel */
    pList = g_list_next(pList);
 
    /* Récupération du GtkLabel */
    pTempLabel = GTK_WIDGET(pList->data);
 
    /* Recuperation du texte contenu dans le GtkEntry */
    sText = gtk_entry_get_text(GTK_ENTRY(pTempEntry));
 
    /* Modification du texte contenu dans le GtkLabel */
    gtk_label_set_text(GTK_LABEL(pTempLabel), sText);
 
    /* Libération de la mémoire utilisée par la liste */
    g_list_free(pList);
}
