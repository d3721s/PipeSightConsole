package org.pipesight.console;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;

public final class NativeNotification {
    private static final String CHANNEL_ID = "pipesight_alerts";
    private static final String POST_NOTIFICATIONS = "android.permission.POST_NOTIFICATIONS";
    private static final int REQUEST_POST_NOTIFICATIONS = 41001;

    private NativeNotification() {
    }

    public static boolean requestPermission(Context context) {
        if (context == null)
            return false;
        if (Build.VERSION.SDK_INT < 33)
            return true;
        if (context.checkSelfPermission(POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        if (context instanceof Activity) {
            ((Activity) context).requestPermissions(
                    new String[] { POST_NOTIFICATIONS },
                    REQUEST_POST_NOTIFICATIONS);
        }
        return false;
    }

    public static boolean notificationsEnabled(Context context) {
        if (context == null)
            return false;
        NotificationManager manager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (manager == null)
            return false;
        if (Build.VERSION.SDK_INT >= 24)
            return manager.areNotificationsEnabled();
        return true;
    }

    public static boolean show(Context context, String title, String message, int notificationId) {
        if (context == null)
            return false;
        if (!requestPermission(context))
            return false;

        NotificationManager manager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (manager == null)
            return false;

        ensureChannel(manager);

        Intent launchIntent = context.getPackageManager()
                .getLaunchIntentForPackage(context.getPackageName());
        if (launchIntent == null)
            launchIntent = new Intent();
        launchIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);

        int flags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= 23)
            flags |= PendingIntent.FLAG_IMMUTABLE;
        PendingIntent pendingIntent =
                PendingIntent.getActivity(context, 0, launchIntent, flags);

        Notification.Builder builder = Build.VERSION.SDK_INT >= 26
                ? new Notification.Builder(context, CHANNEL_ID)
                : new Notification.Builder(context);

        String safeTitle = title == null || title.length() == 0 ? "PipeSight Console" : title;
        String safeMessage = message == null ? "" : message;

        builder.setSmallIcon(smallIcon(context))
                .setContentTitle(safeTitle)
                .setContentText(safeMessage)
                .setStyle(new Notification.BigTextStyle().bigText(safeMessage))
                .setContentIntent(pendingIntent)
                .setAutoCancel(true)
                .setShowWhen(true)
                .setWhen(System.currentTimeMillis());

        manager.notify(notificationId, builder.build());
        return true;
    }

    private static void ensureChannel(NotificationManager manager) {
        if (Build.VERSION.SDK_INT < 26)
            return;
        NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "PipeSight Alerts",
                NotificationManager.IMPORTANCE_DEFAULT);
        channel.setDescription("PipeSight Console system notifications");
        manager.createNotificationChannel(channel);
    }

    private static int smallIcon(Context context) {
        int icon = context.getApplicationInfo().icon;
        return icon != 0 ? icon : android.R.drawable.ic_dialog_info;
    }
}
