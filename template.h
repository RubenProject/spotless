#ifndef VAR_TEMPLATE_H
#define VAR_TEMPLATE_H

#include "list.h"

#define BROWSE_TEMPLATE "templates/browse_template.html"
#define VIDEO_TEMPLATE "templates/video_template.html"
#define VIDEODIV_TEMPLATE "templates/video_div.html"
#define HOME_TEMPLATE "templates/home_template.html"
#define CATDIV_TEMPLATE "templates/cat_div.html"


void gen_home_html(char **html, struct str_list *list);

void gen_browse_html(const char *path, char **html);

void gen_video_html(const char *path, char **html);



#endif
