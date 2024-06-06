/////// DOESNT WORK FREEZES ON SHUTDOWN ///////

#include <gst/gst.h>
#include <glib.h>
#include <signal.h>
#include <iostream>
#include <thread>
#include <atomic>

GMainLoop *main_loop;
GstElement *main_pipeline, *file_pipeline, *display_pipeline, *udp_pipeline;
std::atomic<bool> terminate_flag(false);

void handle_sigint(int sig) {
    terminate_flag = true;
    g_main_loop_quit(main_loop);
}

void cleanup_and_exit() {
    std::cout << "Sending EOS to all pipelines...\n";
    gst_element_send_event(main_pipeline, gst_event_new_eos());
    gst_element_send_event(file_pipeline, gst_event_new_eos());
    gst_element_send_event(display_pipeline, gst_event_new_eos());
    gst_element_send_event(udp_pipeline, gst_event_new_eos());

    std::cout << "Waiting for EOS messages...\n";
    GstBus *bus;
    GstMessage *msg;

    bus = gst_element_get_bus(main_pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_EOS);
    if (msg != NULL) gst_message_unref(msg);
    gst_object_unref(bus);

    bus = gst_element_get_bus(file_pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_EOS);
    if (msg != NULL) gst_message_unref(msg);
    gst_object_unref(bus);

    bus = gst_element_get_bus(display_pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_EOS);
    if (msg != NULL) gst_message_unref(msg);
    gst_object_unref(bus);

    bus = gst_element_get_bus(udp_pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_EOS);
    if (msg != NULL) gst_message_unref(msg);
    gst_object_unref(bus);

    std::cout << "Setting pipelines to NULL state...\n";
    gst_element_set_state(main_pipeline, GST_STATE_NULL);
    gst_element_set_state(file_pipeline, GST_STATE_NULL);
    gst_element_set_state(display_pipeline, GST_STATE_NULL);
    gst_element_set_state(udp_pipeline, GST_STATE_NULL);

    std::cout << "Unrefing pipelines...\n";
    gst_object_unref(main_pipeline);
    gst_object_unref(file_pipeline);
    gst_object_unref(display_pipeline);
    gst_object_unref(udp_pipeline);

    std::cout << "Unrefing main loop...\n";
    g_main_loop_unref(main_loop);
    std::cout << "Cleanup complete, exiting.\n";
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GError *error = NULL;
    GstStateChangeReturn ret;

    // Set up signal handling for graceful termination
    signal(SIGINT, handle_sigint);

    // Create the main pipeline: Capture video from the camera and encode it to H.264
    main_pipeline = gst_parse_launch(
        "v4l2src ! videoconvert ! x264enc ! tee name=t t. ! queue ! interpipesink name=sink1", &error);
    if (!main_pipeline) {
        g_printerr("Error creating main pipeline: %s\n", error->message);
        g_clear_error(&error);
        return -1;
    }

    // Create the file saving pipeline: Receive the encoded video and save it to a file
    file_pipeline = gst_parse_launch(
        "interpipesrc listen-to=sink1 ! qtmux ! filesink location=output.mp4", &error);
    if (!file_pipeline) {
        g_printerr("Error creating file pipeline: %s\n", error->message);
        g_clear_error(&error);
        return -1;
    }

    // Create the display pipeline: Receive the encoded video and display it on the same computer
    display_pipeline = gst_parse_launch(
        "interpipesrc listen-to=sink1 ! avdec_h264 ! videoconvert ! autovideosink", &error);
    if (!display_pipeline) {
        g_printerr("Error creating display pipeline: %s\n", error->message);
        g_clear_error(&error);
        return -1;
    }

    // Create the UDP streaming pipeline: Receive the encoded video and stream it via UDP
    udp_pipeline = gst_parse_launch(
        "interpipesrc listen-to=sink1 ! udpsink host=127.0.0.1 port=5000", &error);
    if (!udp_pipeline) {
        g_printerr("Error creating UDP pipeline: %s\n", error->message);
        g_clear_error(&error);
        return -1;
    }

    // Set the pipelines to PLAYING state
    ret = gst_element_set_state(main_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the main pipeline to the playing state.\n");
        gst_object_unref(main_pipeline);
        return -1;
    }

    ret = gst_element_set_state(file_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the file pipeline to the playing state.\n");
        gst_object_unref(file_pipeline);
        return -1;
    }

    ret = gst_element_set_state(display_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the display pipeline to the playing state.\n");
        gst_object_unref(display_pipeline);
        return -1;
    }

    ret = gst_element_set_state(udp_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the UDP pipeline to the playing state.\n");
        gst_object_unref(udp_pipeline);
        return -1;
    }

    // Create and run the main loop
    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);

    // Clean up and ensure file is saved correctly
    if (terminate_flag) {
        cleanup_and_exit();
    }

    return 0;
}
