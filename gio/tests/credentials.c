/* GLib testing framework examples and tests
 *
 * Copyright © 2012 Collabora Ltd.
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 */

#include "config.h"

#include <gio/gio.h>
#include <gio/gcredentialsprivate.h>

static void
test_basic (void)
{
  GCredentials *creds = g_credentials_new ();
  GCredentials *other = g_credentials_new ();
#if G_CREDENTIALS_SUPPORTED
  GError *error = NULL;
  gboolean set;
  pid_t not_me;
  gchar *stringified;
#endif

  /* You can always get a credentials object, but it might not work. */
  g_assert (creds != NULL);
  g_assert (other != NULL);

#if G_CREDENTIALS_SUPPORTED
  g_assert (g_credentials_is_same_user (creds, other, &error));
  g_assert_no_error (error);

  if (geteuid () == 0)
    not_me = 65534; /* traditionally 'nobody' */
  else
    not_me = 0;

  g_assert_cmpuint (g_credentials_get_unix_user (creds, &error), ==,
      geteuid ());
  g_assert_no_error (error);
  g_assert_cmpuint (g_credentials_get_unix_pid (creds, &error), ==,
      getpid ());
  g_assert_no_error (error);

  set = g_credentials_set_unix_user (other, not_me, &error);
  g_assert_no_error (error);
  g_assert (set);

  g_assert_cmpuint (g_credentials_get_unix_user (other, &error), ==, not_me);
  g_assert (!g_credentials_is_same_user (creds, other, &error));
  g_assert_no_error (error);

  stringified = g_credentials_to_string (creds);
  g_test_message ("%s", stringified);
  g_free (stringified);

  stringified = g_credentials_to_string (other);
  g_test_message ("%s", stringified);
  g_free (stringified);

#if G_CREDENTIALS_USE_LINUX_UCRED
        {
          struct ucred *native = g_credentials_get_native (creds,
              G_CREDENTIALS_TYPE_LINUX_UCRED);

          g_assert_cmpuint (native->uid, ==, geteuid ());
          g_assert_cmpuint (native->pid, ==, getpid ());
        }
#elif G_CREDENTIALS_USE_FREEBSD_CMSGCRED
        {
          struct cmsgcred *native = g_credentials_get_native (creds,
              G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED);

          g_assert_cmpuint (native->cmcred_euid, ==, geteuid ());
          g_assert_cmpuint (native->cmcred_pid, ==, getpid ());
        }
#elif G_CREDENTIALS_USE_OPENBSD_SOCKPEERCRED
        {
          struct sockpeercred *native = g_credentials_get_native (creds,
              G_CREDENTIALS_TYPE_OPENBSD_SOCKPEERCRED);

          g_assert_cmpuint (native->uid, ==, geteuid ());
          g_assert_cmpuint (native->pid, ==, getpid ());
        }
#else
#error "G_CREDENTIALS_SUPPORTED is set but there is no test for this platform"
#endif

#endif

  g_object_unref (creds);
  g_object_unref (other);
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/credentials/basic", test_basic);

  return g_test_run();
}
