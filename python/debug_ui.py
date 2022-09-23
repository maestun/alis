import dearpygui.dearpygui as dpg

class DebugUI():
    def __init__(self):
        def save_callback():
            print("Save Clicked")

        dpg.create_context()
        dpg.create_viewport()
        dpg.setup_dearpygui()

        with dpg.window(label="alis debugger"):
            dpg.add_text("Hello world")
            dpg.add_button(label="Save", callback=save_callback)
            dpg.add_input_text(label="string")
            dpg.add_slider_float(label="float")
        
        with dpg.window(label="Tutorial"):
            with dpg.table(header_row=False):

                # use add_table_column to add columns to the table,
                # table columns use child slot 0
                dpg.add_table_column()
                dpg.add_table_column()
                dpg.add_table_column()

                # add_table_next_column will jump to the next row
                # once it reaches the end of the columns
                # table next column use slot 1
                for i in range(0, 4):
                    with dpg.table_row():
                        for j in range(0, 3):
                            dpg.add_text(f"Row{i} Column{j}")


        dpg.show_viewport()


    def run(self, vm):
        # below replaces, start_dearpygui()
        while dpg.is_dearpygui_running():
            # insert here any code you would like to run in the render loop
            # you can manually stop by using stop_dearpygui()
            dpg.render_dearpygui_frame()
            vm.tick()

        dpg.destroy_context()
