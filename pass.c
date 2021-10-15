#include <archive.h>
#include <archive_entry.h>
#include <glib.h>
#include <libgen.h>

#define mylog(format, ...) printf(format "\n", ##__VA_ARGS__)
#define myerr(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)
#define TIME 1

static struct archive *setup_archive(gchar *password, ssize_t buffer_size) {
  struct archive *a;

  if ((a = archive_write_new()) == NULL) {
    myerr("Failed to allocate archive for writing");
    goto err;
  }

  if (archive_write_set_format_zip(a) != ARCHIVE_OK) {
    myerr("Failed to set archive write format zip: %s",
          archive_error_string(a));
    goto err;
  }

  if ((archive_write_zip_set_compression_store(a)) != ARCHIVE_OK) {
    myerr("Failed to set archive write zip compression store: %s",
          archive_error_string(a));
    goto err;
  }

  if (archive_write_set_options(a, "zip:encryption=aes256") != ARCHIVE_OK) {
    myerr("Failed to set archive write options: %s", archive_error_string(a));
    goto err;
  }

  if (archive_write_set_passphrase(a, password) != ARCHIVE_OK) {
    myerr("Failed to set archive write passphrase: %s",
          archive_error_string(a));
    goto err;
  }

  goto out;

err:
  archive_write_free(a);
  a = NULL;
out:
  return a;
}

static struct archive_entry *setup_archive_entry(struct archive *a,
                                                 gchar *filename) {
  struct archive_entry *ae;

  if ((ae = archive_entry_new()) == NULL) {
    myerr("Failed to allocate archive entry");
    goto err;
  }

  archive_entry_set_filetype(ae, AE_IFREG);
  archive_entry_set_pathname(ae, basename(filename));
  archive_entry_set_size(ae, G_MAXINT64);

  if (archive_write_header(a, ae) != ARCHIVE_OK) {
    myerr("Failed to archive write header: %s", archive_error_string(a));
    goto err;
  }

  goto out;

err:
  archive_entry_free(ae);
  ae = NULL;
out:
  return ae;
}

static la_ssize_t callback_write(G_GNUC_UNUSED struct archive *a,
                                 G_GNUC_UNUSED void *_client_data,
                                 G_GNUC_UNUSED const void *_buffer,
                                 size_t _length) {
  return _length;
}

static int read_file_write_archive(struct archive *a, gchar *filename,
                                      ssize_t buffer_size) {
  FILE *file = NULL;
  ssize_t bytesread = 0;
  time_t t0, t1, tx = 0;
  gint8 *buffer = g_new0(gint8, buffer_size);
  int retval = 1;

  if ((file = fopen(filename, "rb")) == NULL) {
    myerr("Could not open %s", filename);
    goto out;
  }

  if ((bytesread = fread(buffer, 1, buffer_size, file)) <= 0) {
    myerr("Could not read file");
    fclose(file);
    goto out;
  }
  fclose(file);

  do {
    t0 = time(NULL);
    archive_write_data(a, buffer, bytesread);
    t1 = time(NULL);
    tx += t1 - t0;
  } while (difftime(tx, 0) < TIME);

  retval = 0;

out:
  g_free(buffer);
  return retval;
}

G_GNUC_UNUSED static int archive_open(gchar *input, gchar *output, gchar *password, ssize_t buffer_size) {
  struct archive *a = NULL;
  struct archive_entry *ae = NULL;
  int retval = 1;

  if ((a = setup_archive(password, buffer_size)) == NULL) {
    goto out;
  }

  if (archive_write_open_filename(a, output) != ARCHIVE_OK) {
    myerr("error at archive_write_open_filename: %s", archive_error_string(a));
    goto out;
  }

  if ((ae = setup_archive_entry(a, input)) == NULL) {
    myerr("error at setup_archive_entry: %s", archive_error_string(a));
    goto out;
  }

  retval = read_file_write_archive(a, input, buffer_size);


out:
  archive_entry_free(ae);
  archive_write_free(a);

  return retval;
}

static gchar *gen_pass(ssize_t length, gchar c) {
  gchar *pass = g_new0(gchar, length+1);
  for(size_t i = 0; i < length; i++) {
    pass[i] = c;
  }
  return pass;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    myerr("%s <inputfile> <passphase length>", argv[0]);
    return 1;
  }

  gchar *inputfile = argv[1];
  gint pass_len = g_ascii_strtoll(argv[2], NULL, 10);

  gchar c = 'a';
  gchar *password = gen_pass(pass_len, c);
  gchar *outputfile = g_strdup_printf("./archive%ld.zip", strlen(password));
  archive_open(inputfile, outputfile, password, 16384);
  mylog("created %s with password '%s' (length %ld)", outputfile, password, strlen(password));
  g_free(password);

  return 0;
}
