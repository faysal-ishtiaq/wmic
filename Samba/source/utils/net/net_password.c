/* 
   Samba Unix/Linux SMB client library 
   Distributed SMB/CIFS Server Management Utility 

   Copyright (C) 2004 Stefan Metzmacher (metze@samba.org)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "includes.h"
#include "utils/net/net.h"
#include "libnet/libnet.h"
#include "system/filesys.h"
#include "auth/credentials/credentials.h"

/*
 * Code for Changing and setting a password
 */

static int net_password_change_usage(struct net_context *ctx, int argc, const char **argv)
{
	d_printf("net_password_change_usage: TODO\n");
	return 0;	
}


static int net_password_change(struct net_context *ctx, int argc, const char **argv)
{
	NTSTATUS status;
	struct libnet_context *libnetctx;
	union libnet_ChangePassword r;
	char *password_prompt = NULL;
	const char *new_password;

	if (argc > 0 && argv[0]) {
		new_password = argv[0];
	} else {
		password_prompt = talloc_asprintf(ctx->mem_ctx, "Enter new password for account [%s\\%s]:", 
							cli_credentials_get_domain(ctx->credentials), 
							cli_credentials_get_username(ctx->credentials));
		new_password = getpass(password_prompt);
	}

	libnetctx = libnet_context_init(NULL);
	if (!libnetctx) {
		return -1;	
	}
	libnetctx->cred = ctx->credentials;

	/* prepare password change */
	r.generic.level			= LIBNET_CHANGE_PASSWORD_GENERIC;
	r.generic.in.account_name	= cli_credentials_get_username(ctx->credentials);
	r.generic.in.domain_name	= cli_credentials_get_domain(ctx->credentials);
	r.generic.in.oldpassword	= cli_credentials_get_password(ctx->credentials);
	r.generic.in.newpassword	= new_password;

	/* do password change */
	status = libnet_ChangePassword(libnetctx, ctx->mem_ctx, &r);
	if (!NT_STATUS_IS_OK(status)) {
		DEBUG(0,("net_password_change: %s\n",r.generic.out.error_string));
		return -1;
	}

	talloc_free(libnetctx);

	return 0;
}


static int net_password_set_usage(struct net_context *ctx, int argc, const char **argv)
{
	d_printf("net_password_set_usage: TODO\n");
	return 0;	
}


static int net_password_set(struct net_context *ctx, int argc, const char **argv)
{
	NTSTATUS status;
	struct libnet_context *libnetctx;
	union libnet_SetPassword r;
	char *password_prompt = NULL;
	char *p;
	char *tmp;
	const char *account_name;
	const char *domain_name;
	const char *new_password = NULL;

	switch (argc) {
		case 0: /* no args -> fail */
			return net_password_set_usage(ctx, argc, argv);
		case 1: /* only DOM\\user; prompt for password */
			tmp = talloc_strdup(ctx->mem_ctx, argv[0]);
			break;
		case 2: /* DOM\\USER and password */
			tmp = talloc_strdup(ctx->mem_ctx, argv[0]);
			new_password = argv[1];
			break;
		default: /* too mayn args -> fail */
			DEBUG(0,("net_password_set: too many args [%d]\n",argc));
			return net_password_usage(ctx, argc, argv);
	}

	if ((p = strchr_m(tmp,'\\'))) {
		*p = 0;
		domain_name = tmp;
		account_name = talloc_strdup(ctx->mem_ctx, p+1);
	} else {
		account_name = tmp;
		domain_name = cli_credentials_get_domain(ctx->credentials);
	}

	if (!new_password) {
		password_prompt = talloc_asprintf(ctx->mem_ctx, "Enter new password for account [%s\\%s]:", 
							domain_name, account_name);
		new_password = getpass(password_prompt);
	}

	libnetctx = libnet_context_init(NULL);
	if (!libnetctx) {
		return -1;	
	}
	libnetctx->cred = ctx->credentials;

	/* prepare password change */
	r.generic.level			= LIBNET_SET_PASSWORD_GENERIC;
	r.generic.in.account_name	= account_name;
	r.generic.in.domain_name	= domain_name;
	r.generic.in.newpassword	= new_password;

	/* do password change */
	status = libnet_SetPassword(libnetctx, ctx->mem_ctx, &r);
	if (!NT_STATUS_IS_OK(status)) {
		DEBUG(0,("net_password_set: %s\n",r.generic.out.error_string));
		return -1;
	}

	talloc_free(libnetctx);

	return 0;
}


static const struct net_functable net_password_functable[] = {
	{"change", "change password (old password required)\n", net_password_change, net_password_change_usage },
	{"set", "set password\n", net_password_set, net_password_set_usage },
	{NULL, NULL}
};

int net_password(struct net_context *ctx, int argc, const char **argv) 
{
	return net_run_function(ctx, argc, argv, net_password_functable, net_password_usage);
}

int net_password_usage(struct net_context *ctx, int argc, const char **argv)
{
	d_printf("net password <command> [options]\n");
	return 0;	
}
