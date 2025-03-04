
# The $log_rotate_type must be set to 'auto' or 'manual' in main test file
assert($log_rotate_type);

SET @old_max_size = @@global.audit_log_filter_max_size;

SELECT audit_log_filter_set_filter('log_all', '{"filter": {"log": true}}');
SELECT audit_log_filter_set_user('%', 'log_all');

# max_size is not set, no logs prune happens
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc

# stop updating log before size check
SELECT audit_log_filter_remove_user('%');

if ($log_rotate_type == 'manual') {
--replace_column 1 <file_name>
SELECT audit_log_rotate();
}

--let $log_path = `SELECT @@global.datadir`
--let $log_name = `SELECT @@global.audit_log_filter_file`
--let $log_max_size = 8192

--source check_logs_size.inc

# logs get pruned after max_size update
SET GLOBAL audit_log_filter_max_size = 8192;

--source check_logs_size.inc

# overall logs size should not exceed max_size while writing audit events
SELECT audit_log_filter_set_user('%', 'log_all');
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc

# stop updating log before size check
SELECT audit_log_filter_remove_user('%');

if ($log_rotate_type == 'manual') {
--replace_column 1 <file_name>
SELECT audit_log_rotate();
}

--source check_logs_size.inc

--echo #
--echo # Cleanup
DELETE FROM mysql.audit_log_user;
DELETE FROM mysql.audit_log_filter;
SELECT audit_log_filter_flush();

SET GLOBAL audit_log_filter_max_size = @old_max_size;
