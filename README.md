apache-mod-wplogin
==================

An Apache module that prevents naive brute force attempts on WordPress.


Why?
----

wp-login.php is a common target for brute force attacks.  The typical bot will submit
POST requests to this page repeatedly, hoping that a bad password has been picked for
a common username (typically admin).  This Apache module blocks these attacks by requiring
a special cookie.

When a legitimate user goes to wp-login.php without this cookie, the module will redirect
them to a page that uses JavaScript to set this special cookie and the return them to
the login form.  Because most bots do not interpret and execute JavaScript, they will never
receive the cookie.  So, when they attempt to submit a POST request to wp-login.php without
the cookie, they'll get back a 401 Unauthorized response from Apache.

This prevents these bots from eating up server resources attempting to log into WordPress
without requiring you to maintain or check any server-side state like log files.  And it
blocks the request early in the process so that blocking them does not require expensive
resources like WordPress database queries, PHP execution, etc.

This approach also has the benefit that it will work for all WordPress sites hosted on a
server, rather than requiring them to be individually updated.


Building the Module
-------------------

You'll first need to install the Apache development packages for your distro (e.g. httpd-devel
on CentOS).  Then, you can use the apxs tool to build the module:

    sudo apxs -i -a -c mod_wplogin.c

apxs should also add the module to your httpd.conf automatically.  So, after building you can
restart Apache.  To test, go to wp-login.php.  Following the redirects, you should see an
extra cookie called "wplogin" with the value "authorized".  Attempts to POST to wp-login.php
without this cookie present should return a 401 response.

You can test the plugin with curl like this:

    curl -s -v -L -d "pwd=your_password" -d "log=your_username" -d "wp-submit=Log%20In" http://example.org/wp-login.php

You should see the 401 response in the output.

