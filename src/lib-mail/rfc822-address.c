/* Copyright (C) 2002 Timo Sirainen */

#include "lib.h"
#include "str.h"
#include "rfc822-tokenize.h"
#include "rfc822-address.h"

static Rfc822Address *new_address(Pool pool, Rfc822Address ***next_addr)
{
	Rfc822Address *addr;

	addr = p_new(pool, Rfc822Address, 1);

	**next_addr = addr;
	*next_addr = &addr->next;

	return addr;
}

Rfc822Address *rfc822_address_parse(Pool pool, const unsigned char *data,
				    size_t size)
{
	static const Rfc822Token stop_tokens_init[] =
		{ ',', '@', '<', ':', TOKEN_LAST };
	static const Rfc822Token stop_tokens_group[] =
		{ ',', '@', '<', ';', TOKEN_LAST };
	static const Rfc822Token stop_tokens_domain[] =
		{ ',', '<', TOKEN_LAST };
	static const Rfc822Token stop_tokens_domain_group[] =
		{ ',', '<', ';', TOKEN_LAST };
	static const Rfc822Token stop_tokens_post_addr[] =
		{ ',', TOKEN_LAST };
	static const Rfc822Token stop_tokens_post_addr_group[] =
		{ ',', ';', TOKEN_LAST };
	static const Rfc822Token stop_tokens_addr_route[] =
		{ ':', '>', TOKEN_LAST };
	static const Rfc822Token stop_tokens_addr_mailbox[] =
		{ '@', '>', TOKEN_LAST };
	static const Rfc822Token stop_tokens_addr_domain[] =
		{ '>', TOKEN_LAST };

	Rfc822Address *first_addr, **next_addr, *addr;
	Rfc822TokenizeContext *ctx;
	const Rfc822Token *stop_tokens;
	Rfc822Token token;
	String *mailbox, *domain, *route, *name, *comment, *next_phrase;
	size_t len;
	int ingroup, stop;

	if (size == 0)
		return NULL;

	first_addr = NULL;
	next_addr = &first_addr;

	/* 1) name <@route:mailbox@domain>, ...
	   2) mailbox@domain (name), ...
	   3) group: name <box@domain>, box2@domain2 (name2), ... ;, ...

	   ENVELOPE wants groups to be stored like (NIL, NIL, group, NIL),
	   ..., (NIL, NIL, NIL, NIL)
	*/
	ctx = rfc822_tokenize_init(data, size, NULL, NULL);
	rfc822_tokenize_skip_comments(ctx, FALSE);

	t_push();
	mailbox = t_str_new(128);
	domain = t_str_new(256);
	route = t_str_new(128);
	name = t_str_new(256);
	comment = t_str_new(256);

	ingroup = FALSE; len = 0;
	stop_tokens = stop_tokens_init;

	next_phrase = mailbox; stop = FALSE;
	while (!stop) {
		if (next_phrase == name && str_len(name) > 0) {
			/* continuing previously started name,
			   separate it from us with space */
			str_append_c(name, ' ');
			len = str_len(name);
		} else {
			len = 0;
		}
		rfc822_tokenize_get_string(ctx, next_phrase, comment,
					   stop_tokens);

		if (next_phrase == name && len > 0 && len == str_len(name)) {
			/* nothing appeneded, remove the space */
			str_truncate(name, len-1);
		}

		token = rfc822_tokenize_get(ctx);
		switch (token) {
		case TOKEN_LAST:
		case ',':
		case ';':
			/* end of address */
			if (str_len(mailbox) > 0 || str_len(domain) > 0 ||
			    str_len(route) > 0 || str_len(name) > 0) {
				addr = new_address(pool, &next_addr);
				addr->mailbox = p_strdup(pool, str_c(mailbox));
				addr->domain = str_len(domain) == 0 ? NULL :
					p_strdup(pool, str_c(domain));
				addr->route = str_len(route) == 0 ? NULL :
					p_strdup(pool, str_c(route));
				addr->name = next_phrase == name ?
					p_strdup(pool, str_c(name)) :
					p_strdup(pool, str_c(comment));
			}

			if (ingroup && token == ';') {
				/* end of group - add end of group marker */
				ingroup = FALSE;
				(void)new_address(pool, &next_addr);
			}

			if (token == TOKEN_LAST) {
				stop = TRUE;
				break;
			}

			stop_tokens = ingroup ? stop_tokens_group :
				stop_tokens_init;

			str_truncate(mailbox, 0);
			str_truncate(domain, 0);
			str_truncate(route, 0);
			str_truncate(name, 0);
			str_truncate(comment, 0);

			next_phrase = mailbox;
			break;
		case '@':
			/* domain part comes next */
			next_phrase = domain;
			stop_tokens = ingroup ? stop_tokens_domain_group :
				stop_tokens_domain;
			break;
		case '<':
			/* route-addr */

			/* mailbox/domain name so far has actually
			   been the real name */
			str_append_str(name, mailbox);
			str_truncate(mailbox, 0);

			if (str_len(domain) > 0) {
                                str_append_c(name, '@');
				str_append_str(name, domain);
				str_truncate(domain, 0);
			}

			/* mailbox */
			rfc822_tokenize_get_string(ctx, mailbox, NULL,
						   stop_tokens_addr_mailbox);

			if (rfc822_tokenize_get(ctx) == '@' &&
			    str_len(mailbox) == 0) {
				/* route is given */
				rfc822_tokenize_get_string(ctx,
					route, NULL, stop_tokens_addr_route);

				if (rfc822_tokenize_get(ctx) == ':') {
					/* mailbox comes next */
					rfc822_tokenize_get_string(ctx,
						mailbox, NULL,
						stop_tokens_addr_mailbox);
				}
			}

			if (rfc822_tokenize_get(ctx) == '@') {
				/* domain */
				rfc822_tokenize_get_string(ctx,
					domain, NULL, stop_tokens_addr_domain);
			}

			token = rfc822_tokenize_get(ctx);
			i_assert(token == '>' || token == TOKEN_LAST);

			next_phrase = name;
			stop_tokens = ingroup ? stop_tokens_post_addr_group :
				stop_tokens_post_addr;
			break;
		case ':':
			/* beginning of group */
			addr = new_address(pool, &next_addr);
			addr->name = p_strdup(pool, str_c(mailbox));

			str_truncate(mailbox, 0);
			str_truncate(comment, 0);

			ingroup = TRUE;
			stop_tokens = stop_tokens_group;
			break;
		default:
			i_unreached();
			break;
		}
	}

	if (ingroup)
		(void)new_address(pool, &next_addr);

	t_pop();
	rfc822_tokenize_deinit(ctx);

	return first_addr;
}

