#ifndef __MBOX_INDEX_H
#define __MBOX_INDEX_H

#include "md5.h"
#include "mail-index.h"

typedef struct {
	MailIndex *index;
	MailFlags flags;
	const char **custom_flags;
	MD5Context md5;
	int received;

	IStream *input;
	uoff_t content_length;
	int set_read_limit;
} MboxHeaderContext;

int mbox_set_syscall_error(MailIndex *index, const char *function);

/* Make sure the mbox is opened. If reopen is TRUE, the file is closed first,
   which is useful when you want to be sure you're not accessing a deleted
   mbox file. */
int mbox_file_open(MailIndex *index);
IStream *mbox_get_stream(MailIndex *index, uoff_t offset,
			 MailLockType lock_type);
void mbox_file_close_stream(MailIndex *index);
void mbox_file_close_fd(MailIndex *index);

void mbox_header_init_context(MboxHeaderContext *ctx, MailIndex *index,
			      IStream *input);
void mbox_header_free_context(MboxHeaderContext *ctx);
void mbox_header_func(MessagePart *part __attr_unused__,
		      const unsigned char *name, size_t name_len,
		      const unsigned char *value, size_t value_len,
		      void *context);
void mbox_keywords_parse(const unsigned char *value, size_t len,
			 const char *custom_flags[MAIL_CUSTOM_FLAGS_COUNT],
			 void (*func)(const unsigned char *, size_t,
				      int, void *),
			 void *context);
int mbox_skip_crlf(IStream *input);
void mbox_skip_empty_lines(IStream *input);
void mbox_skip_header(IStream *input);
void mbox_skip_message(IStream *input);
int mbox_verify_end_of_body(IStream *input, uoff_t end_offset);
int mbox_mail_get_location(MailIndex *index, MailIndexRecord *rec,
			   uoff_t *offset, uoff_t *hdr_size, uoff_t *body_size);

MailIndex *mbox_index_alloc(const char *dir, const char *mbox_path);
int mbox_index_rebuild(MailIndex *index);
int mbox_index_sync(MailIndex *index, MailLockType lock_type, int *changes);
int mbox_sync_full(MailIndex *index);
IStream *mbox_open_mail(MailIndex *index, MailIndexRecord *rec,
			time_t *internal_date, int *deleted);

int mbox_index_append(MailIndex *index, IStream *input);

time_t mbox_from_parse_date(const unsigned char *msg, size_t size);
const char *mbox_from_create(const char *sender, time_t time);

int mbox_index_rewrite(MailIndex *index);

#endif
