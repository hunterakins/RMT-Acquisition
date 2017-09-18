#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>


int main(int argc, char **argv)
{
	int cas;
	config_t cfg;
	config_setting_t *setting;

	config_init(&cfg);
	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "config.cfg")) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}


	setting = config_lookup(&cfg, "cascade");
	printf("%d\n", sizeof(setting));
	//unsigned int l = config_setting_length(setting);
	//printf("%d\n", l);
		
	cas = config_setting_get_bool_elem(setting, 0);	
	
	printf("%d\n", cas);
	config_destroy(&cfg);
	return(EXIT_SUCCESS);
}

