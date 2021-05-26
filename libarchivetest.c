#include <archive.h>
#include <archive_entry.h>
#include <glib.h>
#include <libgen.h>

#define mylog(format, ...) printf(format "\n", ##__VA_ARGS__)
#define myerr(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)
#define TIME 3

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

  if (archive_write_set_bytes_per_block(a, buffer_size) != ARCHIVE_OK) {
    myerr("Failed to set blocksize: %s", archive_error_string(a));
    goto out;
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

static double read_file_write_archive(struct archive *a, gchar *filename,
                                      ssize_t buffer_size) {
  FILE *file = NULL;
  ssize_t bytesread = 0;
  ssize_t filesize = 0;
  ssize_t counter = 0;
  double throughput = 0;
  double diff = 0;
  time_t t0, t1, tx = 0;
  gint8 *buffer = g_new0(gint8, buffer_size);

  printf("Doing archive aes-256 for %ds on %zd size blocks:", TIME,
         buffer_size);
  fflush(stdout);

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
    filesize += bytesread;
    t0 = time(NULL);
    archive_write_data(a, buffer, bytesread);
    t1 = time(NULL);
    tx += t1 - t0;
    counter++;
  } while ((diff = difftime(tx, 0)) < TIME);

  throughput = filesize / (diff * 1000);
  mylog("\rDoing archive aes-256 for %ds on %zd size blocks: %zd archive "
        "aes-256 in %.2lfs",
        TIME, buffer_size, counter, diff);

out:
  g_free(buffer);
  return throughput;
}

G_GNUC_UNUSED static double archive_open(gchar *filename, ssize_t buffer_size) {
  double throughput = 0;
  gchar *password = "pass";
  struct archive *a = NULL;
  struct archive_entry *ae = NULL;

  if ((a = setup_archive(password, buffer_size)) == NULL) {
    goto out;
  }

  if (archive_write_open(a, NULL, NULL, callback_write, NULL) != ARCHIVE_OK) {
    goto out;
  }

  if ((ae = setup_archive_entry(a, filename)) == NULL) {
    goto out;
  }

  throughput = read_file_write_archive(a, filename, buffer_size);

out:
  archive_entry_free(ae);
  archive_write_free(a);
  return throughput;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    myerr("%s <file>", argv[0]);
    return 1;
  }

  gchar *filename = argv[1];
  int buffer_sizes[] = {16, 64, 256, 1024, 8192, 16384};
  ssize_t buffer_len = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);
  double res[buffer_len];
  gchar *result_str = g_strdup("archive");

  for (ssize_t i = 0; i < buffer_len; i++) {
    res[i] = archive_open(filename, buffer_sizes[i]);
    gchar *tmp = result_str;
    result_str = g_strdup_printf("%s\t%.2lfk", tmp, res[i]);
    g_free(tmp);
  }

  mylog("The 'numbers' are in 1000s of bytes per second processed.");
  mylog("type\t16 bytes\t64 bytes\t256 bytes\t1024 bytes\t8192 bytes\t16384 "
        "bytes");
  mylog("%s", result_str);
  g_free(result_str);

  return 0;
}
