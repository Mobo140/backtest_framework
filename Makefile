BUILD_DIR := build
CMAKE_BUILD_TYPE := Debug
LOB ?= MD/lob.csv
TRADES ?= MD/trades.csv
MAX_EVENTS ?= 5000
TARGET_POSITION ?= 20
ORDER_QTY ?= 1
REPORT_DIR ?= results
SWEEP_ROOT ?= results/sweeps
PYTHON ?= python3

MAX_POSITIONS := 20 50 100 200 400
HORIZON_SPECS := 10k:10000 50k:50000 100k:100000 1b:1000000

BT := ./$(BUILD_DIR)/bt_runner
BT_BASE_ARGS := --lob $(LOB) --trades $(TRADES)

.PHONY: configure build run test clean rebuild \
        run-sweep analyze-sweep experiments visualize

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) -j

run: build
	./$(BUILD_DIR)/bt_runner \
		--lob $(LOB) \
		--trades $(TRADES) \
		--max-events $(MAX_EVENTS) \
		--target-position $(TARGET_POSITION) \
		--order-qty $(ORDER_QTY) \
		--report-dir $(REPORT_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# ── experiments ───────────────────────────────────────────────────────────
run-sweep: build
	@for spec in $(HORIZON_SPECS); do \
		horizon=$${spec%%:*}; \
		events=$${spec##*:}; \
		for max_pos in $(MAX_POSITIONS); do \
			for run in A B C; do \
				report_dir="$(SWEEP_ROOT)/max_pos_$${max_pos}/$${horizon}/run_$${run}"; \
				mkdir -p "$${report_dir}"; \
				echo ">>> max_pos=$${max_pos} horizon=$${horizon} events=$${events} run=$${run}"; \
				if [ "$${run}" = "A" ]; then \
					$(BT) $(BT_BASE_ARGS) --max-events "$${events}" --report-dir "$${report_dir}" --no-as-microprice --as-max-position "$${max_pos}"; \
				elif [ "$${run}" = "B" ]; then \
					$(BT) $(BT_BASE_ARGS) --max-events "$${events}" --report-dir "$${report_dir}" --as-microprice --as-max-position "$${max_pos}"; \
				else \
					$(BT) $(BT_BASE_ARGS) --max-events "$${events}" --report-dir "$${report_dir}" --as-microprice --as-2018-spread --as-max-position "$${max_pos}"; \
				fi; \
			done; \
		done; \
	done

analyze-sweep:
	cd results && $(PYTHON) analyze.py

experiments: run-sweep analyze-sweep

visualize: analyze-sweep

# ── misc ──────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build
