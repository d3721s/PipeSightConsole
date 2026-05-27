package org.pipesight.console;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Color;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

public final class NativeSystemUi {
    private NativeSystemUi() {
    }

    public static boolean applyImmersiveStatusBar(Context context) {
        Activity activity = activityFrom(context);
        if (activity == null)
            return false;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Window window = activity.getWindow();
                if (window == null)
                    return;

                window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
                window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
                window.setStatusBarColor(Color.TRANSPARENT);

                if (Build.VERSION.SDK_INT >= 30)
                    window.setDecorFitsSystemWindows(true);

                View decorView = window.getDecorView();
                int flags = View.SYSTEM_UI_FLAG_LAYOUT_STABLE;

                if (Build.VERSION.SDK_INT >= 23) {
                    // Dark app theme: keep status-bar icons light.
                    flags &= ~View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
                }

                decorView.setSystemUiVisibility(flags);
            }
        });
        return true;
    }

    private static Activity activityFrom(Context context) {
        Context current = context;
        while (current instanceof ContextWrapper) {
            if (current instanceof Activity)
                return (Activity) current;
            current = ((ContextWrapper) current).getBaseContext();
        }
        return null;
    }
}
