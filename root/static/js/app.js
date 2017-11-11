;(function() {
    /// View model of users page
    var viewmodel = {
        /// Registered users model
        entries:   window["__initdata"].entries,
  
        /// Is model update is performing
        busy: false,
  
        /// Display notification
        messagevisible: true,
  
        /// Selected count
        sel_count: function() {
            return viewmodel.entries
                            .filter(function (r) { return r.selected === true })
                            .length;
        },
  
        /// Registered watchers count
        watchers_cnt: function () {
            return viewmodel.entries
                            .filter(function (r) { return r.is_admin === 0 })
                            .length;
        },
  
        /// Active users count
        watchers_actcnt: function () {
            return viewmodel.entries
                            .filter(function (r) { 
                                return r.is_admin === 0 && 
                                       r.active   === 1 &&
                                      (r.valid_to ? 
                                            new Date(r.valid_to) > 
                                            new Date() : 
                                            true)
                                })
                            .length;
        },
  
        // Active modal window,
        modal: "",
  
        /// Close modal window
        modalClose: function () {
            viewmodel.modal = "";
            viewmodel.multiEditor.entries.length = 0;
        },
  
        // Single user edit model
        singleUser: {
            mode:      "",
            title:     "",
  
            username:  "",
            password:  "",
            active:    false,
            is_admin:  false,
            valid_to: "",
    
            valid_msg: function() {
                var su  = viewmodel.singleUser,
                    msg = [];
                    
                switch (su.mode) {
                    case "create":
                        if (!/^[\w\d\_\-]{1,20}$/g.test(su.username)) {
                           msg.push("Имя пользователя может содержать только " +
                                    "до 20 латинскиих символов и цифр");
                        }
                        
                        if (su.password.length === 0) {
                           msg.push("Требуется указать пароль");
                        }
                        
                        if (su.active && su.valid_to) {
                            var activeToDate = new Date(su.valid_to),
                                now          = new Date();
                                
                            if (activeToDate < now) {
                                msg.push("Укажите срок действия позже "+
                                         "сегодняшнего дня");
                            }
                        }
                    break;
                    
                    case "modify":
                        if (su.active && su.valid_to) {
                            var activeToDate = new Date(su.valid_to),
                                now          = new Date();
                                
                            if (activeToDate < now) {
                                msg.push("Укажите срок действия позже "+
                                         "сегодняшнего дня");
                            }
                        }
                    break;
                }
                
                return msg;
            },
  
            // Perform model validation and store model
            proceed: function() {
                var su = viewmodel.singleUser,
                    entity = [
                        "username=" + encodeURIComponent(su.username),
                        "active="   + (su.active    ? "1" : "0"),
                        "is_admin=" + (su.is_admin  ? "1" : "0"),
                        "valid_to=" + (su.valid_to  ? 
                                            encodeURIComponent(su.valid_to) : 
                                            "null"),
                    ];
                    
                if (su.valid_msg().length > 0)
                    return;
                    
                if (su.password)
                    entity.push("password=" + encodeURIComponent(su.password));
                
                viewmodel.modalClose();
                viewmodel.busy = true;
                
                nanoajax.ajax({
                    url: su.mode === 'create' ? 
                                '/users/create' : 
                                '/users/modify', 
                    method: 'POST', 
                    body: entity.join('&')
                }, function (code, responseText, request) {
                    viewmodel.busy = false;
                    var rdata = JSON.parse(responseText);
                    
                    if (rdata.success)
                        viewmodel.entries = rdata.model;
                    
                    scope.scan();
                });
            }
        },
  
        // Multiple user creation dialog
        multiEditor: {
            // Dialog state
            state:    "",
  
            // Dialog message
            message: "",
  
            // Username prefix
            prefix:   "",
  
            /// Information about created users
            entries: [],
  
            // Should users be active at creation
            active:   false,
  
            // When users should be active
            valid_to: "",
            
            // Count of users to create
            create_cnt: 0,
  
            /// Model validation message
            valid_msg: function() {
                var vm = viewmodel.multiEditor,
                    r = [];
                    
                if (vm.create_cnt < 0)
                    r.push("Можно создать как минимум одного пользователя");
                
                if (vm.create_cnt > 10)
                    r.push("Можно создать не более 10 записей за раз");
                
                if (!(/^[\w\d\-\_]{0,10}$/g.test(vm.prefix)))
                    r.push("Префикс должен содержать не более 10 символов "+
                            "латинского алфавита или цифр");
                    
                return r;
            },
  
            // Create and send command
            proceed: function() {
                var vm = viewmodel.multiEditor;
                
                if (vm.valid_msg().length !== 0)
                    return;
                
                var postdata = [];
                
                postdata.push("valid_to=" + 
                    (vm.valid_to ? encodeURIComponent(vm.valid_to) : "null"));
                postdata.push("active=" + (vm.active ? "1" : "0"));
                postdata.push("create_cnt=" + 
                    parseInt(vm.create_cnt, 10).toString());
                postdata.push("prefix=" + encodeURIComponent(vm.prefix));
                
                vm.state = "performing";
                viewmodel.busy  = true;
                
                nanoajax.ajax({
                    url:    '/users/createMultiple', 
                    method: 'POST', 
                    body:   postdata.join("&")
                }, function (code, responseText, request) {
                    vm.state = "results";
                    
                    var rdata = JSON.parse(responseText);
                    
                    if (rdata.success) {
                        viewmodel.entries = rdata.model;
                        vm.entries = rdata.entries;
                    }
                    
                    vm.message = rdata.message;
                    viewmodel.busy = false;
                    
                    scope.scan();
                });
            }
        },
  
        // User active period editor
        dateEditor: {
            // Account expiration date
            targetDate: "",
  
            // Check validation message
            valid_msg: function() {
                var m = viewmodel.dateEditor;
                
                if (m.targetDate) {
                    var target = new Date(m.targetDate),
                        now    = new Date();
                        
                    if (target < now)
                        return "Укажите дату позже сегодняшнего дня";
                }
                
                return "";
            },
  
            // Apply changes
            proceed: function() {
                var m = viewmodel.dateEditor;
                
                if (m.valid_msg().length !== 0)
                    return;
                
                viewmodel.modalClose();
                
                viewmodel.busy = true;
                
                nanoajax.ajax({
                    url: '/users/activate', 
                    method: 'POST', 
                    body:  "valid_to=" + (m.targetDate ? 
                                            encodeURIComponent(m.targetDate) : 
                                            "null") + 
                           "&" +
                            "users=" + viewmodel
                                        .entries
                                        .filter(function(q) {return q.selected})
                                        .map(function(q) { return q.username })
                                        .join(",")
                }, function (code, responseText, request) {
                    viewmodel.busy = false;
                    var rdata = JSON.parse(responseText);
                    
                    if (rdata.success)
                        viewmodel.entries = rdata.model;
                    
                    scope.scan();
                });
            }
        },
  
        // Create single user
        createSingle: function() {
            var su = viewmodel.singleUser;
            su.username  = "";
            su.password  = "";
            su.active    = false;
            su.is_admin  = false;
            su.active_to = null;
            su.mode      = "create";
            su.title     = "Создание пользователя";
            
            viewmodel.modal = "singleEditor";
        },
  
        // Create multiple users
        createMultiple: function() {
            var me = viewmodel.multiEditor;
            me.state = "enter";
            me.entries.length = 0;
            viewmodel.modal = "multiEditor";
        },
  
        // Modify entity
        modifyEntity: function(entity) {
            var su = viewmodel.singleUser;
            
            su.username  = entity.username;
            su.password  = "";
            su.active    = entity.active   === 1;
            su.is_admin  = entity.is_admin === 1;
            su.active_to = entity.active_to ? new Date(entity.active_to) : null;
            su.mode      = "modify";
            su.title     = "Редактирование пользователя";
            
            viewmodel.modal = "singleEditor";
        },
  
        // Open date editor
        modifyDate: function() {
            viewmodel.modal = "dateEditor";
            viewmodel.dateEditor.targetDate = "";
        },
  
        // Remove inactive users from database
        removeInactive: function() {
            viewmodel.busy = true;
                
            nanoajax.ajax({
                url: '/users/removeInactive', 
                method: 'POST', 
                body: ""
            }, function (code, responseText, request) {
                viewmodel.busy = false;
                var rdata = JSON.parse(responseText);
                
                if (rdata.success)
                    viewmodel.entries = rdata.model;
                
                scope.scan();
            });
        },
  
        // Remove all users except active one
        removeAllExceptMe: function() {
            viewmodel.busy = true;
                
            nanoajax.ajax({
                url: '/users/removeAll', 
                method: 'POST', 
                body: ""
            }, function (code, responseText, request) {
                viewmodel.busy = false;
                var rdata = JSON.parse(responseText);
                
                if (rdata.success)
                    viewmodel.entries = rdata.model;
                
                scope.scan();
            });
        },
  
        // Remove selected users except active one
        removeSelected: function() {
            viewmodel.busy = true;
                
            nanoajax.ajax({
                url: '/users/remove', 
                method: 'POST', 
                body: "users=" + viewmodel
                                    .entries
                                    .filter(function(q) { return q.selected })
                                    .map(function(q) { return q.username })
                                    .join(",")
            }, function (code, responseText, request) {
                viewmodel.busy = false;
                var rdata = JSON.parse(responseText);
                
                if (rdata.success)
                    viewmodel.entries = rdata.model;
                
                scope.scan();
            });
        },
  
        // Modify entity activeness
        activateSelection: function(activation) {
            viewmodel.busy = true;
                
            nanoajax.ajax({
                url: '/users/activate', 
                method: 'POST', 
                body:  "activate=" + (activation ? '1&' : '0&' ) +
                       "users=" + viewmodel
                                    .entries
                                    .filter(function(q) { return q.selected })
                                    .map(function(q) { return q.username })
                                    .join(",")
            }, function (code, responseText, request) {
                viewmodel.busy = false;
                var rdata = JSON.parse(responseText);
                
                if (rdata.success)
                    viewmodel.entries = rdata.model;
                
                scope.scan();
            });
        }
    };
    
    var scope = alight.ChangeDetector(viewmodel);
    
    alight.utils.pars_start_tag = '{!';
    alight.utils.pars_finish_tag = '!}';
    
    alight('#application', scope);
})();
